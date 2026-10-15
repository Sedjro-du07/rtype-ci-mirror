# ENG-37 · network_asio plugin: UDP and TCP on an I/O thread

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **10 Network** · **Wave 4** · status and owner live on the board.

## Goal

The transport plugin: UDP and TCP on an I/O thread with standalone Asio (ADR 0037: TCP from Part 1).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/network_asio/CMakeLists.txt`
- `plugins/network_asio/AsioNetwork.hpp`
- `plugins/network_asio/AsioNetwork.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `plugins/network_asio/AsioNetwork.hpp`

```cpp
class AsioNetwork final : public INetwork {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "asio"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;       // starts the I/O thread
    void shutdown() noexcept override;                          // stops io_context, joins the thread
    // UDP
    Result<void> openUdp(uint16_t port) override;
    void closeUdp() override;
    void sendUdp(const Endpoint& to, std::span<const std::byte> bytes) override;
    void setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) override;
    // TCP
    Result<void> listenTcp(uint16_t port) override;
    Result<uint32_t> connectTcp(const Endpoint& to) override;
    void sendTcp(uint32_t stream, std::span<const std::byte> bytes) override;
    void setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)> fn) override;
    void closeTcp(uint32_t stream) override;
private:
    void startReceive();                                        // async_receive_from loop
    void startAccept();                                         // async_accept loop
    void startRead(uint32_t stream);                            // async_read_some loop
    asio::io_context                                     io_;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_;
    std::thread                                          thread_;
    asio::ip::udp::socket                                udp_;
    std::optional<asio::ip::tcp::acceptor>               acceptor_;
    std::unordered_map<uint32_t, asio::ip::tcp::socket>  streams_;
};
ENGINE_PLUGIN(AsioNetwork)
```

## Depends on

- [ENG-36](ENG-36.md) — INetwork and network types
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- [SRV-06](SRV-06.md) — StreamFramer: TCP message framing with size limit

## Read first

- [ADR 0033](../adr/0033-engine-networking.md)
- [ADR 0037](../adr/0037-game-facing-networking.md)

## To implement

- [ ] One `io_context` on its own thread
- [ ] UDP socket: open, send (thread-safe copy), receiver callback
- [ ] TCP acceptor and streams: listen, connect, send, receiver, close

## Done when

- Loopback test: datagrams and stream bytes arrive; closing during traffic does not crash

## Watch out

- The receiver runs on the I/O thread: only push into the session inbox

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
