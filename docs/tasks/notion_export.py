#!/usr/bin/env python3
"""Turn generated task pages (docs/tasks/<ID>.md) into the content of their Notion page.

Run from the repo root:  python3 docs/tasks/notion_export.py ENG-09 SRV-08 ...  > out.json
Prints {id: {"page_id", "wave", "properties", "content"}}. Links between tasks become bold IDs (the
task is on the board), other links point to GitHub, the title and the Workflow section are dropped.
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(__file__))
import generate as g  # noqa: E402

GITHUB = 'https://github.com/EpitechPGE3-2026/G-ING-500-COT-5-1-rtype-2/blob/architecture-task-distribution/docs/tasks/'
IDS = json.load(open(os.path.join(os.path.dirname(__file__), 'notion_ids.json')))


def content(tid):
    md = open(os.path.join(g.ROOT, 'docs/tasks', f'{tid}.md')).read()
    md = md.split('\n', 2)[2]                                   # drop the title (it is the Task property)
    md = md.split('\n## Workflow')[0].rstrip() + '\n'
    md = re.sub(r'^Board: .*$', f'Repo copy: [{tid}.md]({GITHUB}{tid}.md) · Wave **{g.WAVES[tid]}** · '
                'status and owner are the properties above.', md, count=1, flags=re.M)
    md = re.sub(r'\[((?:ENG|SRV)-\d\d)\]\([^)]+\)', r'**\1**', md)

    def absolute(m):
        text, url = m.group(1), m.group(2)
        if url.startswith('http'):
            return m.group(0)
        return f'[{text}]({os.path.normpath(GITHUB + url).replace("https:/", "https://")})'
    md = re.sub(r'\[([^\]]+)\]\(([^)]+)\)', absolute, md)
    md = md.replace('Relative to `engine/`. Each file already exists as a skeleton naming this task.',
                    'Relative to `engine/`; each file exists as a skeleton naming this task.')
    # outside code: wrap template names (on<T>) in backticks so Notion does not read them as tags
    parts = re.split(r'(```.*?```)', md, flags=re.S)
    for i in range(0, len(parts), 2):
        segs = parts[i].split('`')
        for j in range(0, len(segs), 2):
            segs[j] = re.sub(r'(\b[\w:]+<[^<>\s]+>(?:\(\))?)', r'`\1`', segs[j])
        parts[i] = '`'.join(segs)
    return ''.join(parts)


def properties(tid):
    t = g.BY_ID[tid]
    p = {'Task': t['title'], 'userDefined:ID': tid, 'Phase': t['phase'], 'Depends on': ', '.join(t['deps']) or '—',
         'File': ' · '.join(t['files'] + ['/' + f for f in t.get('root_files', [])]), 'Ref': t['ref'], 'Wave': g.WAVES[tid]}
    if t['board'] == 'server':
        p['Details'] = GITHUB + tid + '.md'
    return p


if __name__ == '__main__':
    out = {}
    for tid in sys.argv[1:]:
        out[tid] = {'page_id': IDS.get(tid), 'board': g.BY_ID[tid]['board'],
                    'properties': properties(tid), 'content': content(tid)}
    print(json.dumps(out, ensure_ascii=False, indent=1))
