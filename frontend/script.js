function animateUpdate(id, value) {
  const el = document.getElementById(id);
  if (el.textContent != value) {
    el.textContent = value;
    el.classList.add('highlight');
    setTimeout(() => el.classList.remove('highlight'), 400);
  }
}

async function updateData() {
  try {
    const res = await fetch('/status');
    const data = await res.json();
    animateUpdate('steps', data.steps);
    animateUpdate('tokens', data.tokens);
  } catch (err) {
    console.error("Eroare la fetch:", err);
  }
}

updateData();
setInterval(updateData, 1000);

// Dark mode logic
const toggle = document.getElementById('darkToggle');
const modeLabel = document.getElementById('modeLabel');
const logo = document.getElementById('logoImage');

function applyTheme(isDark) {
  if (isDark) {
    document.body.classList.add('dark');
    modeLabel.textContent = '🌞 Light Mode';
    logo.src = 'icon2.png';
    localStorage.setItem('theme', 'dark');
    toggle.checked = true;
  } else {
    document.body.classList.remove('dark');
    modeLabel.textContent = '🌙 Dark Mode';
    logo.src = 'icon.png';
    localStorage.setItem('theme', 'light');
    toggle.checked = false;
  }
}

toggle.addEventListener('change', () => {
  applyTheme(toggle.checked);
});

// Apply saved theme on load
const savedTheme = localStorage.getItem('theme');
applyTheme(savedTheme === 'dark');
