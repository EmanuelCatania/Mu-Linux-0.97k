/**
* Updates and displays the current Server Time (Europe/Sofia timezone)
* and the User's Local Time every second.
*/

document.addEventListener('DOMContentLoaded', () => {
  const serverTimeEl = document.getElementById('server-time');
  const userTimeEl = document.getElementById('user-time');
  const serverFormatter = new Intl.DateTimeFormat('en-US', {
    timeZone: 'Europe/Sofia',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: false
  });

  const pad = n => String(n).padStart(2, '0');
  const formatLocalTime = date => `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;

  const updateTimes = () => {
    const now = new Date();
    serverTimeEl.textContent = serverFormatter.format(now);
    userTimeEl.textContent = formatLocalTime(now);
  };

  updateTimes();
  setInterval(updateTimes, 1000);
});