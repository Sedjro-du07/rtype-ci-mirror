-- engine/tools/wireshark/msep.lua
-- SRV-19 — Wireshark dissector for MSEP v1.0 (Magic Shifter Engine Protocol)
-- Spec: docs/network/rfc.md · network step 7
--
-- Install: copy this file to your Wireshark plugins folder, or load it via
--   Edit > Preferences > Plugins, or add  dofile("/path/to/msep.lua")  to
--   your init.lua.  Then filter with  msep  in the display-filter bar.
--
-- Heuristic: UDP/TCP data that starts with magic 0x5254 and version 0x01.

-- ---------------------------------------------------------------------------
-- Protocol and field declarations
-- ---------------------------------------------------------------------------

local msep = Proto("msep", "Magic Shifter Engine Protocol")

-- Header (UDP + TCP preamble)
local f_magic     = ProtoField.uint16("msep.magic",     "Magic",       base.HEX)
local f_version   = ProtoField.uint8 ("msep.version",   "Version",     base.DEC)
-- UDP-specific header fields
local f_kind      = ProtoField.uint8 ("msep.kind",      "Kind",        base.HEX,
                        {[0x01]="UdpLink", [0x02]="Data"})
local f_session   = ProtoField.uint32("msep.sessionKey","Session Key",  base.HEX)
local f_seq       = ProtoField.uint16("msep.sequence",  "Sequence",    base.DEC)
local f_ack       = ProtoField.uint16("msep.ack",       "Ack",         base.DEC)
local f_ackbits   = ProtoField.uint32("msep.ackBits",   "Ack Bits",    base.HEX)
-- UdpLink extra field
local f_linktoken = ProtoField.uint64("msep.linkToken", "Link Token",  base.HEX)
-- TCP framing
local f_framelength = ProtoField.uint32("msep.frameLength", "Frame Length", base.DEC)
-- Message envelope
local f_msg_id    = ProtoField.uint16("msep.msg.id",    "Message Id",  base.HEX)
local f_msg_seq   = ProtoField.uint16("msep.msg.seq",   "Message Seq (reliable)", base.DEC)
local f_msg_len   = ProtoField.uint16("msep.msg.length","Payload Length", base.DEC)
-- Hello payload
local f_hello_name      = ProtoField.string("msep.hello.name",      "Player Name")
local f_hello_fp        = ProtoField.uint64("msep.hello.fingerprint","Fingerprint",  base.HEX)
local f_hello_game      = ProtoField.string("msep.hello.gameName",   "Game Name")
local f_hello_token     = ProtoField.bytes ("msep.hello.reconnectToken", "Reconnect Token")
local f_hello_dataid    = ProtoField.uint16("msep.hello.gameDataId", "Game Data Id", base.HEX)
-- Challenge payload
local f_chal_token      = ProtoField.uint64("msep.challenge.linkToken", "Link Token", base.HEX)
-- Accepted payload
local f_acc_playerid    = ProtoField.uint32("msep.accepted.playerId",   "Player Id",     base.DEC)
local f_acc_sesskey     = ProtoField.uint32("msep.accepted.sessionKey", "Session Key",   base.HEX)
local f_acc_token       = ProtoField.bytes ("msep.accepted.reconnectToken", "Reconnect Token")
local f_acc_resumed     = ProtoField.bool  ("msep.accepted.resumed",    "Resumed")
local f_acc_tickrate    = ProtoField.uint16("msep.accepted.tickRate",    "Tick Rate",     base.DEC)
local f_acc_snaprate    = ProtoField.uint16("msep.accepted.snapshotRate","Snapshot Rate", base.DEC)
local f_acc_heartbeat   = ProtoField.uint16("msep.accepted.heartbeat",   "Heartbeat (ms)",base.DEC)
local f_acc_timeout     = ProtoField.uint16("msep.accepted.timeout",     "Timeout (ms)",  base.DEC)
local f_acc_grace       = ProtoField.uint16("msep.accepted.grace",       "Grace (s)",     base.DEC)
-- Refused payload
local f_ref_reason      = ProtoField.uint8 ("msep.refused.reason",   "Reason",  base.DEC,
                            {[0]="VersionMismatch",[1]="FormatMismatch",[2]="TooManyPending",
                             [3]="InvalidName",[4]="WrongGame",[5]="ServerFull",
                             [6]="Timeout",[7]="RefusedByGame"})
local f_ref_msg         = ProtoField.string("msep.refused.message",  "Message")
-- Snapshot payload
local f_snap_roomsess   = ProtoField.uint16("msep.snapshot.roomSession",        "Room Session", base.DEC)
local f_snap_tick       = ProtoField.uint32("msep.snapshot.tick",               "Tick",         base.DEC)
local f_snap_lastinput  = ProtoField.uint32("msep.snapshot.lastInputProcessed", "Last Input",   base.DEC)
local f_snap_count      = ProtoField.uint16("msep.snapshot.count",              "Entry Count",  base.DEC)
local f_snap_netid      = ProtoField.uint32("msep.snapshot.entry.networkId",    "Network Id",   base.DEC)
local f_snap_op         = ProtoField.uint8 ("msep.snapshot.entry.op",           "Op",           base.DEC,
                            {[0]="Update",[1]="Spawn",[2]="Destroy"})
local f_snap_prefab     = ProtoField.uint32("msep.snapshot.entry.prefabKey",    "Prefab Key",   base.HEX)
local f_snap_ncomp      = ProtoField.uint8 ("msep.snapshot.entry.componentCount","Component Count",base.DEC)
local f_snap_compidx    = ProtoField.uint16("msep.snapshot.component.index",    "Component Index",base.DEC)
local f_snap_complen    = ProtoField.uint16("msep.snapshot.component.length",   "Component Length",base.DEC)
local f_snap_compdata   = ProtoField.bytes ("msep.snapshot.component.data",     "Component Data")
-- Input payload
local f_input_roomsess  = ProtoField.uint16("msep.input.roomSession", "Room Session", base.DEC)
local f_input_newest    = ProtoField.uint32("msep.input.newestSeq",   "Newest Seq",   base.DEC)
local f_input_count     = ProtoField.uint8 ("msep.input.count",       "Count",        base.DEC)
local f_input_mask      = ProtoField.uint64("msep.input.mask",        "Action Mask",  base.HEX)

msep.fields = {
    f_magic, f_version, f_kind, f_session, f_seq, f_ack, f_ackbits, f_linktoken,
    f_framelength,
    f_msg_id, f_msg_seq, f_msg_len,
    f_hello_name, f_hello_fp, f_hello_game, f_hello_token, f_hello_dataid,
    f_chal_token,
    f_acc_playerid, f_acc_sesskey, f_acc_token, f_acc_resumed,
    f_acc_tickrate, f_acc_snaprate, f_acc_heartbeat, f_acc_timeout, f_acc_grace,
    f_ref_reason, f_ref_msg,
    f_snap_roomsess, f_snap_tick, f_snap_lastinput, f_snap_count,
    f_snap_netid, f_snap_op, f_snap_prefab, f_snap_ncomp,
    f_snap_compidx, f_snap_complen, f_snap_compdata,
    f_input_roomsess, f_input_newest, f_input_count, f_input_mask,
}

-- ---------------------------------------------------------------------------
-- Message id → name table  (RFC §8.5 + Part II R-Type messages)
-- ---------------------------------------------------------------------------

local MSG_NAMES = {
    [0x0001] = "Hello",           [0x0002] = "Challenge",
    [0x0003] = "Accepted",        [0x0004] = "Refused",
    [0x0005] = "Bye",             [0x0006] = "Kicked",
    [0x0007] = "ServerShutdown",  [0x0008] = "KeepAlive",
    [0x0020] = "InviteRequest",   [0x0021] = "InviteReceived",
    [0x0022] = "InviteAnswer",    [0x0023] = "LeaveParty",
    [0x0024] = "PartyUpdated",    [0x0025] = "RoomListRequest",
    [0x0026] = "RoomList",        [0x0027] = "OnlinePlayers",
    [0x0028] = "PlayerOnline",    [0x0029] = "PlayerOffline",
    [0x002A] = "RequestRefused",
    [0x0040] = "EnteredRoom",     [0x0041] = "RoomReady",
    [0x0042] = "ReturnedToLobby", [0x0043] = "RoomInterrupted",
    [0x0044] = "RoomRedirect",
    [0x0060] = "Snapshot",        [0x0061] = "Input",
    -- R-Type game messages (Part II)
    [0x0100] = "JoinInfo",        [0x0101] = "PlayRequest",
    [0x0102] = "JoinRoomRequest", [0x0103] = "LeaveGame",
    [0x0104] = "Chat",            [0x0105] = "WaveCleared",
    [0x0106] = "PlayerKilled",    [0x0107] = "GameResult",
}

-- IDs that carry a messageSeq field (reliable UDP messages, RFC §4.2.3)
local RELIABLE_IDS = {
    [0x0040]=true, [0x0041]=true, [0x0042]=true, [0x0043]=true, [0x0044]=true,
}

-- ---------------------------------------------------------------------------
-- Helpers
-- ---------------------------------------------------------------------------

--- Read a big-endian u16-length-prefixed string from buf at offset.
--- Returns (string_value, new_offset) or (nil, offset) on underrun.
local function read_str(buf, off, limit)
    if buf:len() < off + 2 then return nil, off end
    local slen = buf(off, 2):uint()
    off = off + 2
    if buf:len() < off + slen then return nil, off end
    if limit and slen > limit then return "(too long: " .. slen .. " bytes)", off + slen end
    return buf(off, slen):string(), off + slen
end

--- Decode a single message payload into tree.
local function decode_payload(buf, off, msg_id, msg_len, tree)
    local payload_end = off + msg_len

    if msg_id == 0x0001 then           -- Hello
        local name, o = read_str(buf, off, 16)
        if name then tree:add(f_hello_name, buf(off, o - off), name) end
        off = o
        if buf:len() >= off + 8 then
            tree:add(f_hello_fp, buf(off, 8)) ; off = off + 8
        end
        local game, o2 = read_str(buf, off, 32)
        if game then tree:add(f_hello_game, buf(off, o2 - off), game) end
        off = o2
        if buf:len() >= off + 16 then
            tree:add(f_hello_token, buf(off, 16)) ; off = off + 16
        end
        if buf:len() >= off + 2 then
            tree:add(f_hello_dataid, buf(off, 2)) ; off = off + 2
        end

    elseif msg_id == 0x0002 then       -- Challenge
        if buf:len() >= off + 8 then
            tree:add(f_chal_token, buf(off, 8)) ; off = off + 8
        end

    elseif msg_id == 0x0003 then       -- Accepted
        if buf:len() >= off + 4  then tree:add(f_acc_playerid,  buf(off,  4)); off = off + 4 end
        if buf:len() >= off + 4  then tree:add(f_acc_sesskey,   buf(off,  4)); off = off + 4 end
        if buf:len() >= off + 16 then tree:add(f_acc_token,     buf(off, 16)); off = off + 16 end
        if buf:len() >= off + 1  then tree:add(f_acc_resumed,   buf(off,  1)); off = off + 1 end
        if buf:len() >= off + 2  then tree:add(f_acc_tickrate,  buf(off,  2)); off = off + 2 end
        if buf:len() >= off + 2  then tree:add(f_acc_snaprate,  buf(off,  2)); off = off + 2 end
        if buf:len() >= off + 2  then tree:add(f_acc_heartbeat, buf(off,  2)); off = off + 2 end
        if buf:len() >= off + 2  then tree:add(f_acc_timeout,   buf(off,  2)); off = off + 2 end
        if buf:len() >= off + 2  then tree:add(f_acc_grace,     buf(off,  2)); off = off + 2 end

    elseif msg_id == 0x0004 then       -- Refused
        if buf:len() >= off + 1 then tree:add(f_ref_reason, buf(off, 1)); off = off + 1 end
        local msg, o = read_str(buf, off, 128)
        if msg then tree:add(f_ref_msg, buf(off, o - off), msg) end
        off = o

    elseif msg_id == 0x0060 then       -- Snapshot
        if buf:len() < off + 10 then return end
        tree:add(f_snap_roomsess,  buf(off, 2)); off = off + 2
        tree:add(f_snap_tick,      buf(off, 4)); off = off + 4
        tree:add(f_snap_lastinput, buf(off, 4)); off = off + 4
        if buf:len() < off + 2 then return end
        local count = buf(off, 2):uint()
        tree:add(f_snap_count, buf(off, 2)); off = off + 2
        for _ = 1, count do
            if buf:len() < off + 5 then break end
            local entry = tree:add(msep, buf(off, 0), "Entry")
            entry:add(f_snap_netid, buf(off, 4)); off = off + 4
            local op = buf(off, 1):uint()
            entry:add(f_snap_op,    buf(off, 1)); off = off + 1
            if op == 1 then
                if buf:len() < off + 4 then break end
                entry:add(f_snap_prefab, buf(off, 4)); off = off + 4
            end
            if op == 0 or op == 1 then
                if buf:len() < off + 1 then break end
                local ncomp = buf(off, 1):uint()
                entry:add(f_snap_ncomp, buf(off, 1)); off = off + 1
                for _ = 1, ncomp do
                    if buf:len() < off + 4 then break end
                    local cidx = buf(off, 2):uint()
                    local clen = buf(off + 2, 2):uint()
                    local comp = entry:add(msep, buf(off, 4 + clen), "Component[" .. cidx .. "]")
                    comp:add(f_snap_compidx,  buf(off,     2)); off = off + 2
                    comp:add(f_snap_complen,  buf(off,     2)); off = off + 2
                    if clen > 0 and buf:len() >= off + clen then
                        comp:add(f_snap_compdata, buf(off, clen)); off = off + clen
                    end
                end
            end
        end

    elseif msg_id == 0x0061 then       -- Input
        if buf:len() < off + 7 then return end
        tree:add(f_input_roomsess, buf(off, 2)); off = off + 2
        tree:add(f_input_newest,   buf(off, 4)); off = off + 4
        local cnt = buf(off, 1):uint()
        tree:add(f_input_count,    buf(off, 1)); off = off + 1
        for _ = 1, cnt do
            if buf:len() < off + 8 then break end
            tree:add(f_input_mask, buf(off, 8)); off = off + 8
        end
    end

    -- Remaining undecoded bytes (game messages or unimplemented engine messages)
    if off < payload_end and buf:len() >= payload_end then
        tree:add(msep, buf(off, payload_end - off), "[payload bytes]")
    end
end

-- ---------------------------------------------------------------------------
-- UDP dissector
-- ---------------------------------------------------------------------------

local function dissect_udp(buf, pinfo, tree)
    local total = buf:len()
    if total < 16 then return false end
    if buf(0, 2):uint() ~= 0x5254 then return false end
    if buf(2, 1):uint() ~= 0x01   then return false end

    pinfo.cols.protocol:set("MSEP")

    local root = tree:add(msep, buf(), "MSEP (UDP)")
    local hdr  = root:add(msep, buf(0, 16), "UDP Header")
    hdr:add(f_magic,   buf(0, 2))
    hdr:add(f_version, buf(2, 1))
    local kind = buf(3, 1):uint()
    hdr:add(f_kind,    buf(3, 1))

    if kind == 0x01 then    -- UdpLink
        pinfo.cols.info:set("UdpLink")
        hdr:add(f_session,   buf(4, 4))
        hdr:add(f_seq,       buf(8, 2))
        hdr:add(f_ack,       buf(10, 2))
        hdr:add(f_ackbits,   buf(12, 4))
        if total >= 24 then root:add(f_linktoken, buf(16, 8)) end
        return true
    end

    if kind ~= 0x02 then return true end   -- unknown kind, still claim the packet

    -- Data packet
    hdr:add(f_session,  buf(4, 4))
    hdr:add(f_seq,      buf(8, 2))
    hdr:add(f_ack,      buf(10, 2))
    hdr:add(f_ackbits,  buf(12, 4))

    local off = 16
    local msg_list = {}

    while off + 4 <= total do
        local msg_id  = buf(off, 2):uint()
        local is_rel  = RELIABLE_IDS[msg_id] or (msg_id >= 0x0100)
        local hdr_sz  = is_rel and 6 or 4   -- id(2) [+ messageSeq(2)] + length(2)
        if off + hdr_sz > total then break end

        local msg_seq = nil
        local len_off = off + 2
        if is_rel then
            msg_seq = buf(off + 2, 2):uint()
            len_off = off + 4
        end
        local msg_len = buf(len_off, 2):uint()
        if off + hdr_sz + msg_len > total then break end

        local name = MSG_NAMES[msg_id] or string.format("0x%04X", msg_id)
        local msub = root:add(msep, buf(off, hdr_sz + msg_len), "Message: " .. name)
        msub:add(f_msg_id,  buf(off, 2))
        if msg_seq then msub:add(f_msg_seq, buf(off + 2, 2)) end
        msub:add(f_msg_len, buf(len_off, 2))

        decode_payload(buf, off + hdr_sz, msg_id, msg_len, msub)
        table.insert(msg_list, name)
        off = off + hdr_sz + msg_len
    end

    if #msg_list == 0 then
        pinfo.cols.info:set("Heartbeat (Data, no messages)")
    else
        pinfo.cols.info:set("Data: " .. table.concat(msg_list, ", "))
    end
    return true
end

-- ---------------------------------------------------------------------------
-- TCP dissector (called by the stream reassembler)
-- ---------------------------------------------------------------------------

-- Per-stream state: have we consumed the 3-byte preamble?
local tcp_preamble_done = {}

local function dissect_tcp(buf, pinfo, tree)
    local total = buf:len()
    if total < 3 then return 0 end   -- need at least the preamble
    if buf(0, 2):uint() ~= 0x5254 then return 0 end
    if buf(2, 1):uint() ~= 0x01   then return 0 end

    pinfo.cols.protocol:set("MSEP")

    local root = tree:add(msep, buf(), "MSEP (TCP)")
    local off  = 0

    -- Preamble (present at the start of each reassembled PDU by the heuristic)
    root:add(f_magic,   buf(0, 2))
    root:add(f_version, buf(2, 1))
    off = 3

    local msg_list = {}

    -- Consume frames until we run out of data
    while off + 6 <= total do   -- 4 (length) + 2 (id) minimum
        local frame_len = buf(off, 4):uint()
        if frame_len < 2 then
            root:add_expert_info(PI_MALFORMED, PI_ERROR,
                "Frame length " .. frame_len .. " < 2")
            break
        end
        -- frame_len covers id(2) + payload
        local full_frame = 4 + frame_len
        if off + full_frame > total then
            -- Need more data — tell the TCP reassembler
            pdu_length = off + full_frame
            break
        end

        local msg_id  = buf(off + 4, 2):uint()
        local pay_len = frame_len - 2
        local name    = MSG_NAMES[msg_id] or string.format("0x%04X", msg_id)
        local msub    = root:add(msep, buf(off, full_frame), "Frame: " .. name)
        msub:add(f_framelength, buf(off, 4))
        msub:add(f_msg_id,      buf(off + 4, 2))
        msub:add(f_msg_len,     buf(off + 4, 2), pay_len) -- pay_len derived from frame_len

        decode_payload(buf, off + 6, msg_id, pay_len, msub)
        table.insert(msg_list, name)
        off = off + full_frame
    end

    if #msg_list > 0 then
        pinfo.cols.info:set("TCP: " .. table.concat(msg_list, ", "))
    end
    return off
end

-- ---------------------------------------------------------------------------
-- Main dissector (heuristic dispatcher)
-- ---------------------------------------------------------------------------

function msep.dissector(buffer, pinfo, tree)
    -- Try TCP framing first (has length prefix after preamble)
    local r = dissect_tcp(buffer, pinfo, tree)
    if r > 0 then return r end
    -- Try UDP packet
    if dissect_udp(buffer, pinfo, tree) then return buffer:len() end
    return 0
end

-- ---------------------------------------------------------------------------
-- Heuristic registration (UDP and TCP)
-- ---------------------------------------------------------------------------

local function heuristic(buffer, pinfo, tree)
    if buffer:len() < 3 then return false end
    if buffer(0, 2):uint() ~= 0x5254 then return false end
    if buffer(2, 1):uint() ~= 0x01   then return false end
    msep.dissector(buffer, pinfo, tree)
    return true
end

msep:register_heuristic("udp", heuristic)
msep:register_heuristic("tcp", heuristic)
