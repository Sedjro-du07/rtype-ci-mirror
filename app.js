const pages = document.getElementById("pages");
const content = document.getElementById("content");
const search = document.getElementById("search");
const results = document.getElementById("results");
const status = document.getElementById("status");
const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)");

let docs = [];
let currentFile = null;
let bodies = null;

async function init() {
  const res = await fetch("docs/index.json");
  docs = await res.json();
  buildSidebar();
  const start = decodeURIComponent(location.hash.slice(1));
  loadDoc(start || docs[0].file, false);
  window.addEventListener("hashchange", () => {
    const file = decodeURIComponent(location.hash.slice(1));
    if (file && file !== currentFile) loadDoc(file, true);
  });
}

function sectionOf(doc) {
  return doc.section || "Other";
}

function link(file, text) {
  const a = document.createElement("a");
  a.className = "nav-item";
  a.href = "#" + file;
  a.textContent = text;
  a.dataset.file = file;
  return a;
}

function buildSidebar() {
  pages.innerHTML = "";
  const sections = new Map();
  docs.forEach(doc => {
    if (!sections.has(sectionOf(doc))) sections.set(sectionOf(doc), []);
    sections.get(sectionOf(doc)).push(doc);
  });
  sections.forEach((list, name) => {
    const heading = document.createElement("h2");
    heading.textContent = name;
    const ul = document.createElement("ul");
    list.forEach(doc => {
      const li = document.createElement("li");
      li.appendChild(link(doc.file, doc.title));
      ul.appendChild(li);
    });
    pages.append(heading, ul);
  });
}

// "engine/guide/README.md" + "../reference/01-core.md" → "engine/reference/01-core.md"
function resolvePath(fromFile, href) {
  const parts = fromFile.split("/").slice(0, -1);
  for (const seg of href.split("/")) {
    if (seg === "..") parts.pop();
    else if (seg !== "." && seg !== "") parts.push(seg);
  }
  return parts.join("/");
}

function wireLinks(fromFile) {
  content.querySelectorAll("a[href]").forEach(a => {
    const href = a.getAttribute("href");
    if (/^[a-z]+:|^#|^\//i.test(href)) return;
    const [path, anchor] = href.split("#");
    if (!path.endsWith(".md")) return;
    const target = resolvePath(fromFile, path);
    a.setAttribute("href", "#" + target);
    a.onclick = (e) => {
      e.preventDefault();
      loadDoc(target, true).then(() => {
        if (anchor) document.getElementById(anchor)?.scrollIntoView();
      });
    };
  });
}

function addHeadingIds() {
  content.querySelectorAll("h1, h2, h3, h4").forEach(h => {
    if (!h.id) h.id = h.textContent.toLowerCase().trim().replace(/[^\w\- ]+/g, "").replace(/\s+/g, "-");
  });
}

async function loadDoc(file, focus) {
  const res = await fetch(`docs/${file}`);
  if (!res.ok) {
    content.innerHTML = `<p>Page not found: <code>docs/${file}</code></p>`;
    return;
  }
  content.innerHTML = marked.parse(await res.text());
  currentFile = file;
  if (location.hash.slice(1) !== file) history.replaceState(null, "", "#" + file);
  const title = docs.find(d => d.file === file)?.title;
  document.title = title ? `${title} · R-Type documentation` : "R-Type documentation";
  addHeadingIds();
  wireLinks(file);
  highlightActive(file);
  addCopyButtons();
  content.scrollTop = 0;
  if (focus) content.focus();
}

function highlightActive(activeFile) {
  pages.querySelectorAll(".nav-item").forEach(a => {
    if (a.dataset.file === activeFile) a.setAttribute("aria-current", "page");
    else a.removeAttribute("aria-current");
  });
}

function addCopyButtons() {
  content.querySelectorAll("pre").forEach(block => {
    if (block.querySelector(".copy-btn")) return;
    const button = document.createElement("button");
    button.type = "button";
    button.textContent = "Copy";
    button.className = "copy-btn";
    button.setAttribute("aria-label", "Copy this code");
    button.onclick = (e) => {
      navigator.clipboard.writeText(block.querySelector("code")?.innerText ?? block.innerText);
      button.textContent = "Copied!";
      spawnConfetti(e.clientX, e.clientY);
      setTimeout(() => (button.textContent = "Copy"), 1500);
    };
    block.appendChild(button);
  });
}

async function loadBodies() {
  if (bodies) return bodies;
  status.textContent = "Indexing pages…";
  bodies = await Promise.all(docs.map(async doc => {
    const res = await fetch(`docs/${doc.file}`);
    return { doc, text: res.ok ? (await res.text()).toLowerCase() : "" };
  }));
  return bodies;
}

function snippet(text, at) {
  const start = Math.max(0, at - 40);
  return (start > 0 ? "…" : "") + text.slice(start, at + 60).replace(/\s+/g, " ") + "…";
}

async function runSearch() {
  const query = search.value.trim().toLowerCase();
  results.innerHTML = "";
  if (query.length < 2) {
    status.textContent = "";
    return;
  }
  const words = query.split(/\s+/);
  const found = (await loadBodies())
    .map(({ doc, text }) => {
      const title = doc.title.toLowerCase();
      if (!words.every(w => text.includes(w) || title.includes(w))) return null;
      const score = words.reduce((s, w) => s + (title.includes(w) ? 10 : 0) + text.split(w).length - 1, 0);
      return { doc, score, at: text.indexOf(words[0]), text };
    })
    .filter(Boolean)
    .sort((a, b) => b.score - a.score)
    .slice(0, 20);
  found.forEach(({ doc, at, text }) => {
    const li = document.createElement("li");
    const a = link(doc.file, doc.title);
    const small = document.createElement("small");
    small.textContent = at >= 0 ? snippet(text, at) : sectionOf(doc);
    a.appendChild(small);
    a.addEventListener("click", () => { search.value = ""; results.innerHTML = ""; status.textContent = ""; });
    li.appendChild(a);
    results.appendChild(li);
  });
  status.textContent = `${found.length} page${found.length === 1 ? "" : "s"} found`;
}

function moveInResults(step) {
  const items = [...results.querySelectorAll("a")];
  if (!items.length) return;
  const index = items.indexOf(document.activeElement);
  items[(index + step + items.length) % items.length].focus();
}

search.addEventListener("input", runSearch);
document.addEventListener("keydown", (e) => {
  const typing = ["INPUT", "TEXTAREA"].includes(document.activeElement.tagName);
  if (e.key === "/" && !typing) {
    e.preventDefault();
    search.focus();
    return;
  }
  if (e.key === "Escape" && (typing || results.contains(document.activeElement))) {
    search.value = "";
    runSearch();
    search.focus();
    return;
  }
  if (e.key === "ArrowDown" && (document.activeElement === search || results.contains(document.activeElement))) {
    e.preventDefault();
    moveInResults(1);
    return;
  }
  if (e.key === "ArrowUp" && results.contains(document.activeElement)) {
    e.preventDefault();
    moveInResults(-1);
  }
});

const themeButton = document.getElementById("theme");
function applyTheme(theme) {
  if (theme) document.documentElement.dataset.theme = theme;
}
try { applyTheme(localStorage.getItem("docs-theme")); } catch (e) {}
themeButton.addEventListener("click", () => {
  const dark = getComputedStyle(document.documentElement).getPropertyValue("--bg").trim() !== "#ffffff";
  const next = dark ? "light" : "dark";
  applyTheme(next);
  try { localStorage.setItem("docs-theme", next); } catch (e) {}
});

function spawnConfetti(x, y) {
  if (reducedMotion.matches) return;
  for (let i = 0; i < 20; i++) {
    const confetti = document.createElement("div");
    confetti.setAttribute("aria-hidden", "true");
    Object.assign(confetti.style, {
      position: "fixed", width: "6px", height: "6px", left: x + "px", top: y + "px", borderRadius: "50%",
      background: `hsl(${Math.random() * 360}, 80%, 60%)`, opacity: 1, transition: "all 1s ease-out",
      pointerEvents: "none",
    });
    document.body.appendChild(confetti);
    const dx = (Math.random() - 0.5) * 100;
    const dy = -Math.random() * 100 - 50;
    setTimeout(() => {
      confetti.style.transform = `translate(${dx}px, ${dy}px) rotate(${Math.random() * 360}deg)`;
      confetti.style.opacity = 0;
    }, 10);
    setTimeout(() => confetti.remove(), 1100);
  }
}

document.addEventListener("mousemove", e => {
  if (reducedMotion.matches) return;
  const star = document.createElement("div");
  star.setAttribute("aria-hidden", "true");
  Object.assign(star.style, {
    position: "fixed", width: "4px", height: "4px", background: "var(--accent)", borderRadius: "50%",
    left: e.clientX + "px", top: e.clientY + "px", pointerEvents: "none", opacity: 1, transition: "all 0.5s ease-out",
  });
  document.body.appendChild(star);
  setTimeout(() => {
    star.style.transform = `translate(${Math.random() * 20 - 10}px, ${Math.random() * 20 - 10}px)`;
    star.style.opacity = 0;
  }, 10);
  setTimeout(() => star.remove(), 500);
});

init();
