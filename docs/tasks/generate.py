#!/usr/bin/env python3
"""Regenerate every task-derived file from docs/tasks/tasks.py and docs/tasks/details.py.

Run from the repo root:  python3 docs/tasks/generate.py

Writes:
  docs/tasks/README.md, docs/tasks/<ID>.md      one detail page per task
  docs/engine/tasks.md, docs/server/tasks.md    the claim tables (reference copies of the Notion boards)
  docs/engine/folder-tree.md                    the tree block (text around it is kept)
  engine/**/README.md                           files to create per folder
  engine/**                                     missing skeleton files; header comments of existing ones
Never overwrites code below a skeleton's header comment.
"""
import os
import re
import sys
from collections import defaultdict

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.dirname(__file__))
from tasks import TASKS          # noqa: E402
from details import DETAILS      # noqa: E402
from api import API              # noqa: E402

GITHUB = 'https://github.com/EpitechPGE3-2026/G-ING-500-COT-5-1-rtype-2/blob/architecture-task-distribution'
BOARDS = {
    'engine': dict(name='Engine — folder tree & tasks', title='Engine tasks',
                   notion='https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7', doc='docs/engine/tasks.md'),
    'server': dict(name='Server — folder tree & tasks', title='Server tasks',
                   notion='https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d', doc='docs/server/tasks.md'),
}
TEAM = 'Ola, Sedjro, Michel, Yann-Ange'
REF_PAGES = {
    '01': '01-core.md', '02': '02-modules.md', '03': '03-ecs.md', '04': '04-reflection.md', '05': '05-scheduler.md',
    '06': '06-events.md', '07': '07-input.md', '08': '08-rendering.md', '09': '09-audio.md', '10': '10-network.md',
    '11': '11-scenes-assets.md', '12': '12-services.md', '13': '13-components-systems.md',
}
FOLDER_SPEC = {
    'core': '01', 'module': '02', 'ecs': '03', 'reflection': '04', 'scheduler': '05', 'events': '06', 'input': '07',
    'render': '08', 'audio': '09', 'net': '10', 'scene': '11', 'assets': '11', 'services': '12', 'script': '12',
    'components': '13', 'systems': '13', 'platform_sfml': '08', 'platform_raylib': '08', 'audio_miniaudio': '09',
    'network_asio': '10', 'scripting_lua': '12', 'launcher': '01', 'cmake': '01', 'module_': '02',
}
BY_ID = {t['id']: t for t in TASKS}


def write(path, text):
    full = os.path.join(ROOT, path)
    os.makedirs(os.path.dirname(full), exist_ok=True)
    old = open(full).read() if os.path.exists(full) else None
    if old != text:
        with open(full, 'w') as f:
            f.write(text)
        print('wrote', path)


def owners():
    """file -> [task ids], in task order; the first one creates the file."""
    out = defaultdict(list)
    for t in TASKS:
        for f in t['files']:
            out[f].append(t['id'])
    return out


def unlocks(tid):
    return [t['id'] for t in TASKS if tid in t['deps']]


def link_task(tid, base):
    return f'[{tid}]({base}{tid}.md)' if tid in BY_ID else tid


def ref_links(ref, base_docs):
    """'ADR 0027, 0036 · reference 04' -> markdown links relative to docs/<base_docs>."""
    out = []
    for adr in re.findall(r'\b(00\d\d)\b', ref):
        name = next((n for n in os.listdir(os.path.join(ROOT, 'docs/adr')) if n.startswith(adr)), None)
        if name:
            out.append(f'[ADR {adr}]({base_docs}adr/{name})')
    for num in re.findall(r'reference (\d\d)', ref):
        out.append(f'[reference {num}]({base_docs}engine/reference/{REF_PAGES[num]})')
    for num in re.findall(r'guide (\d\d)', ref):
        name = next((n for n in os.listdir(os.path.join(ROOT, 'docs/engine/guide')) if n.startswith(num)), None)
        if name:
            out.append(f'[guide {num}]({base_docs}engine/guide/{name})')
    if 'protocol RFC' in ref:
        out.append('protocol RFC (Network part, not written yet)')
    return out


# ------------------------------------------------------------------ classes and methods
def reference_blocks():
    """[(file key, code)] from every reference section whose heading names a file."""
    out = []
    ref = os.path.join(ROOT, 'docs/engine/reference')
    for name in sorted(os.listdir(ref)):
        if not name[:2].isdigit():
            continue
        parts = re.split(r'^(#{2,3} .*)$', open(os.path.join(ref, name)).read(), flags=re.M)
        for i in range(1, len(parts), 2):
            m = re.search(r'— `([^`]+)`', parts[i])
            if not m:
                continue
            key = m.group(1).removeprefix('engine/')
            if key == 'src/net/':                      # private classes are listed per task in api.py
                continue
            for lang, code in re.findall(r'```(cpp|cmake)?\n(.*?)```', parts[i + 1], flags=re.S):
                if re.match(r'\s*void \w+::register', code):     # usage examples are not declarations
                    continue
                out.append((key, lang or 'cpp', code.rstrip()))
    return out


REF_BLOCKS = None


def full_path(key):
    if key.startswith(('cmake/', 'launcher/', 'src/', 'plugins/', 'tests/', 'benchmarks/', 'tools/')) or key.endswith('.cmake'):
        return key
    return 'include/engine/' + key


def api_blocks(t):
    """[(file, lang, code)] grouped by file, declarations only."""
    global REF_BLOCKS
    if REF_BLOCKS is None:
        REF_BLOCKS = reference_blocks()
    spec = API.get(t['id'], {})
    blocks = []
    if spec.get('auto', True):
        wanted = []
        for f in t['files']:
            k = f.removeprefix('include/engine/')
            wanted.append(k)
            if f.endswith('.cpp'):                     # a source implements its public header
                wanted.append(f[:-4].removeprefix('src/') + '.hpp')
        for key, lang, code in REF_BLOCKS:
            if key in spec.get('skip', []):
                continue
            if any(w == key or (key.endswith('/') and w.startswith(key)) for w in wanted):
                blocks.append((full_path(key), lang, code))
    blocks += spec.get('extra', [])
    grouped = {}
    for f, lang, code in blocks:
        g = grouped.setdefault(f, [lang, []])
        if code not in g[1]:
            g[1].append(code)
    return [(f, lang, '\n\n'.join(codes)) for f, (lang, codes) in grouped.items()]


def api_section(t):
    blocks = api_blocks(t)
    note = API.get(t['id'], {}).get('note')
    if not blocks and not note:
        return []
    lines = ['## Classes and methods', '',
             '**Declarations only**: what this task must write, without the bodies. Write the implementation in the '
             'matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the '
             'pages in *Read first*.', '']
    if note:
        lines += [f'> {note}', '']
    for f, lang, code in blocks:
        lines += [f'### `{f}`', '', f'```{lang}', code, '```', '']
    return lines


# ------------------------------------------------------------------ waves
def compute_waves():
    """Wave of a task = 1 + the highest wave among its dependencies (wave 1 = no dependency).
    Tasks of the same wave can be done in parallel; take tasks from the lowest open wave."""
    waves = {}

    def wave(tid, path=()):
        if tid in path:
            sys.exit('dependency cycle: ' + ' -> '.join(path + (tid,)))
        if tid not in waves:
            waves[tid] = 1 + max((wave(d, path + (tid,)) for d in BY_ID[tid]['deps']), default=0)
        return waves[tid]
    for t in TASKS:
        wave(t['id'])
    return waves


WAVES = compute_waves()


def in_wave_order(tasks):
    return sorted(tasks, key=lambda t: (WAVES[t['id']], t['id']))


WAVE_NOTE = ('**Wave** = the order in which tasks can be done: a task\'s wave is one more than the highest wave of '
             'its dependencies, so every task of a wave can be done in parallel once the earlier waves are done. '
             'Take your next task from the lowest open wave.')


# ------------------------------------------------------------------ detail pages
def detail_page(t):
    d = DETAILS.get(t['id'], {})
    board = BOARDS[t['board']]
    lines = [f"# {t['id']} · {t['title']}", '',
             f"Board: **{board['name']}** ([Notion]({board['notion']})) · Phase **{t['phase']}** · "
             f"**Wave {WAVES[t['id']]}** · status and owner live on the board.", '']
    if d.get('goal'):
        lines += ['## Goal', '', d['goal'], '']
    lines += ['## Files', '', 'Relative to `engine/`. Each file already exists as a skeleton naming this task.', '']
    own = owners()
    for f in t['files']:
        note = ''
        others = [x for x in own[f] if x != t['id']]
        if others:
            note = f" (shared with {', '.join(link_task(o, '') for o in others)})"
        lines.append(f'- `{f}`{note}')
    lines.append('')
    lines += api_section(t)
    lines += ['## Depends on', '']
    lines += [f"- {link_task(x, '')} — {BY_ID[x]['title']}" for x in t['deps']] or ['- nothing: can start now']
    lines += ['', '## Unlocks', '']
    lines += [f"- {link_task(x, '')} — {BY_ID[x]['title']}" for x in unlocks(t['id'])] or ['- no task waits for this one']
    lines += ['', '## Read first', '']
    lines += [f'- {x}' for x in ref_links(t['ref'], '../')] or [f"- {t['ref']}"]
    if d.get('todo'):
        lines += ['', '## To implement', ''] + [f'- [ ] {x}' for x in d['todo']]
    if d.get('done'):
        lines += ['', '## Done when', ''] + [f'- {x}' for x in d['done']]
    if d.get('watch'):
        lines += ['', '## Watch out', ''] + [f'- {x}' for x in d['watch']]
    lines += ['', '## Workflow', '',
              '1. Claim the task on the board (Owner, Status = Claimed, Claimed on).',
              '2. Start when every dependency is at least *In review*, or agree on the interface with its owner.',
              '3. Implement, with tests; open a pull request and set *In review*.',
              '4. Merged → *Done* and Completed on.', '']
    return '\n'.join(lines)


def tasks_index():
    lines = ['# Tasks — details', '',
             'One page per task: goal, files, dependencies, what to implement, how to know it is done. '
             'Claiming happens on the Notion boards; this folder is where you read what a task means. '
             'Generated from `tasks.py` and `details.py` by `generate.py`: edit those, not the pages.', '', WAVE_NOTE, '']
    for key, b in BOARDS.items():
        lines += [f"## {b['title']}", '', f"Board: [Notion]({b['notion']}) · table: [`{b['doc']}`](../{b['doc'][5:]})", '',
                  '| Wave | ID | Phase | Task | Depends on |', '|---|---|---|---|---|']
        for t in in_wave_order(TASKS):
            if t['board'] == key:
                deps = ', '.join(link_task(x, '') for x in t['deps']) or '—'
                lines.append(f"| {WAVES[t['id']]} | [{t['id']}]({t['id']}.md) | {t['phase']} | {t['title']} | {deps} |")
        lines.append('')
    return '\n'.join(lines)


# ------------------------------------------------------------------ claim tables
def board_table(key):
    b = BOARDS[key]
    intro = {
        'engine': 'Derived from [`folder-tree.md`](folder-tree.md).',
        'server': 'The server is part of the engine (ADR 0037): its files live in `engine/`, mostly `net/`. '
                  'Decisions: [Notion › 02 · Server](https://app.notion.com/p/3f9efac8d899811da007fd2c3719434f).',
    }[key]
    base = '../tasks/'
    lines = [f"# {b['title']}", '',
             f"{intro} The live board, where you claim tasks, is in Notion (**R-Type — Architecture › {b['name']}**: "
             f"{b['notion']}). This file is the reference copy. Team: {TEAM}.", '',
             'Claim a task yourself on the board: set **Owner** and **Status = Claimed**, and the claim date. '
             'Nobody assigns tasks to anyone else.',
             'Start a task only when the tasks it depends on are at least *In review*, or agree on the interface with their owner first.',
             f'**What each task contains in detail:** click its ID (pages in [`docs/tasks/`]({base}README.md)), or open the task on the board.', '',
             WAVE_NOTE + ' Waves count dependencies across both boards (engine and server).', '',
             '| Wave | ID | Phase | Task | Files | Depends on | Ref |', '|---|---|---|---|---|---|---|']
    for t in in_wave_order(TASKS):
        if t['board'] != key:
            continue
        files = '<br>'.join(f'`engine/{f}`' for f in t['files'])
        deps = ', '.join(t['deps']) or '—'
        lines.append(f"| {WAVES[t['id']]} | [{t['id']}]({base}{t['id']}.md) | {t['phase']} | {t['title']} | {files} | {deps} | {t['ref']} |")
    return '\n'.join(lines) + '\n'


# ------------------------------------------------------------------ folder tree
def tree_block():
    own = owners()
    tree = {}
    for f in sorted(own):
        node = tree
        parts = f.rstrip('/').split('/')
        for p in parts[:-1]:
            node = node.setdefault(p + '/', {})
        leaf = parts[-1] + ('/' if f.endswith('/') else '')
        node.setdefault(leaf, {})
        node[leaf]['__owner__'] = ', '.join(own[f])
    # folders that hold no task file but must appear (kept from the original tree)
    lines = ['engine/']

    def walk(node, prefix):
        ordered = sorted([k for k in node if k != '__owner__'], key=lambda k: (not k.endswith('/'), k.lower()))
        dirs = [k for k in ordered if any(x != '__owner__' for x in node[k])]
        for i, k in enumerate(ordered):
            last = i == len(ordered) - 1
            branch = '└── ' if last else '├── '
            text = prefix + branch + k
            owner = node[k].get('__owner__')
            if owner:
                text = text.ljust(58) + owner
            lines.append(text)
            if k in dirs:
                walk(node[k], prefix + ('    ' if last else '│   '))
    walk(tree, '')
    return '\n'.join(lines)


def folder_tree_doc():
    path = 'docs/engine/folder-tree.md'
    text = open(os.path.join(ROOT, path)).read()
    start = text.index('```\nengine/')
    end = text.index('```', start + 3)
    text = text[:start] + '```\n' + tree_block() + '\n' + text[end:]
    text = re.sub(r'Every file is owned by a\ntask of the engine board.*?first task creates it\.',
                  'Every file is owned by a\ntask: `ENG-xx` on the engine board, `SRV-xx` on the server board (see [`tasks.md`](tasks.md), '
                  '[`../server/tasks.md`](../server/tasks.md) and the Notion boards). A file listed under two tasks is shared: the first '
                  'task creates it. Regenerated by `docs/tasks/generate.py`.', text, flags=re.S)
    return path, text


# ------------------------------------------------------------------ skeletons and READMEs
def spec_for(path):
    parts = path.split('/')
    for p in parts:
        if p in FOLDER_SPEC:
            return f"docs/engine/reference/{REF_PAGES[FOLDER_SPEC[p]]}"
    return 'docs/engine/reference/README.md'


def header(path, tid, comment, spec=None):
    t = BY_ID[tid]
    others = [x for x in owners()[path] if x != tid]
    lines = [f'engine/{path}', f"{tid} — {t['title']}", f"Spec: {spec or spec_for(path)} · {t['ref']}",
             f'Details: docs/tasks/{tid}.md']
    if others:
        lines.append('Also edited by: ' + ', '.join(f"{o} ({BY_ID[o]['title']})" for o in others))
    lines += [f"Board: Notion › R-Type — Architecture › {BOARDS[t['board']]['name']} (self-assign)",
              'Status: not implemented yet. Claim the task on the board before editing this file.']
    return '\n'.join(f'{comment} {x}' for x in lines) + '\n'


def skeleton_body(path):
    name = os.path.basename(path)
    if name.endswith('.hpp'):
        return '\n#pragma once\n\nnamespace engine {\n\n} // namespace engine\n'
    if name.endswith('.cpp'):
        hpp = name[:-4] + '.hpp'
        if path.startswith('src/') and os.path.exists(os.path.join(ROOT, 'engine', os.path.dirname(path), hpp)):
            inc = f'"{hpp}"'                                   # private header next to the source
        else:
            rel = os.path.dirname(path).removeprefix('src/')
            inc = f'"engine/{rel}/{hpp}"'
        return f'\n#include {inc}\n\nnamespace engine {{\n\n}} // namespace engine\n'
    return '\n'


def comment_of(path):
    if path.endswith(('.cmake', 'CMakeLists.txt')):
        return '#'
    if path.endswith('.frag'):
        return '//'
    return '//'


def skeletons():
    own = owners()
    # headers first so .cpp files can detect private headers next to them
    for f in sorted(own, key=lambda p: (not p.endswith('.hpp'), p)):
        full = os.path.join(ROOT, 'engine', f)
        if f.endswith('/'):
            os.makedirs(full, exist_ok=True)
            continue
        c = comment_of(f)
        head = header(f, own[f][0], c)
        if os.path.exists(full):
            old = open(full).read()
            if not old.startswith(f'{c} engine/'):
                continue                                       # real code: leave it alone
            m = re.search(rf'^{re.escape(c)} Spec: (\S+)', old, flags=re.M)
            head = header(f, own[f][0], c, m.group(1) if m else None)
            body_lines = old.split('\n')
            i = 0
            while i < len(body_lines) and body_lines[i].startswith(c + ' '):
                i += 1
            write(f'engine/{f}', head + '\n'.join(body_lines[i:]))
        else:
            write(f'engine/{f}', head + skeleton_body(f))


def folder_readmes():
    own = owners()
    by_dir = defaultdict(list)
    for f in own:
        if f.endswith('/'):
            by_dir[f.rstrip('/')].append(None)
        else:
            by_dir[os.path.dirname(f)].append(f)
    for d, files in by_dir.items():
        path = f'engine/{d}/README.md' if d else 'engine/README.md'
        full = os.path.join(ROOT, path)
        old = open(full).read() if os.path.exists(full) else ''
        title = f'# `engine/{d}/`' if d else '# `engine/`'
        # keep the description paragraph of existing READMEs
        desc = ''
        m = re.match(r'# .*?\n\n(.*?)\n\nPlaceholder', old, flags=re.S)
        if m:
            desc = m.group(1) + '\n\n'
        if d.startswith('src/') and any(f.endswith('.hpp') for f in files if f):
            desc += ('Headers in this folder are **private** to the engine: they are not under `include/`, '
                     'so games cannot include them.\n\n')
        lines = [title, '', desc + 'Placeholder created from the architecture. Replace this file\'s list as files are written.', '']
        real = [f for f in files if f]
        if real:
            lines += ['| File to create | Task | Details |', '|---|---|---|']
            for f in sorted(real, key=str.lower):
                tid = own[f][0]
                extra = ''.join(f', {o}' for o in own[f][1:])
                lines.append(f"| `{os.path.basename(f)}` | {tid}{extra} — {BY_ID[tid]['title']} | `docs/tasks/{tid}.md` |")
        else:
            tid = own[d + '/'][0]
            lines.append(f"Task: {tid} — {BY_ID[tid]['title']} (details: `docs/tasks/{tid}.md`)")
        lines += ['', 'Design: `docs/engine/reference/`. Rules: `docs/engine/folder-tree.md`.', '']
        write(path, '\n'.join(lines))


def main():
    for t in TASKS:
        write(f"docs/tasks/{t['id']}.md", detail_page(t))
    write('docs/tasks/README.md', tasks_index())
    for key, b in BOARDS.items():
        write(b['doc'], board_table(key))
    write(*folder_tree_doc())
    skeletons()
    folder_readmes()
    missing = [t['id'] for t in TASKS if t['id'] not in DETAILS]
    if missing:
        print('no details for:', ', '.join(missing))


if __name__ == '__main__':
    main()
