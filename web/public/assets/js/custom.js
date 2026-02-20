/**
* Updates and displays the current Server Time (configurable timezone)
* and the User's Local Time every second.
*/

document.addEventListener('DOMContentLoaded', () => {
  const serverTimeEl = document.getElementById('server-time');
  const userTimeEl = document.getElementById('user-time');
  const serverTimeZone = serverTimeEl?.dataset?.tz;
  const fallbackTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
  const serverFormatter = new Intl.DateTimeFormat('es-ES', {
    timeZone: serverTimeZone || fallbackTimeZone,
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: false
  });

  const pad = n => String(n).padStart(2, '0');
  const formatLocalTime = date => `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;

  const updateTimes = () => {
    if (!serverTimeEl || !userTimeEl) return;
    const now = new Date();
    serverTimeEl.textContent = serverFormatter.format(now);
    userTimeEl.textContent = formatLocalTime(now);
  };

  updateTimes();
  if (serverTimeEl && userTimeEl) {
    setInterval(updateTimes, 1000);
  }

  const countdownEls = Array.from(document.querySelectorAll('[data-countdown]'));
  const formatCountdown = (totalSeconds) => {
    if (totalSeconds <= 0) return 'En curso';
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;
    return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}`;
  };

  const updateCountdowns = () => {
    if (countdownEls.length === 0) return;
    const now = Date.now();
    countdownEls.forEach((el) => {
      const target = Date.parse(el.getAttribute('data-countdown'));
      if (Number.isNaN(target)) return;
      const diffSeconds = Math.floor((target - now) / 1000);
      el.textContent = formatCountdown(diffSeconds);
    });
  };

  updateCountdowns();
  if (countdownEls.length > 0) {
    setInterval(updateCountdowns, 1000);
  }
});
