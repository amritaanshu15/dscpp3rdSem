/*
 * app.js - main part of the frontend
 *   - login and logout
 *   - changing between pages (#/dashboard, #/population, ...)
 *   - small helper functions used by all pages
 */

// ----- helper functions -----

function byId(id) {
  return document.getElementById(id);
}

// Names typed by the user must not be put into the page as HTML,
// so special characters are replaced first.
function esc(text) {
  return String(text)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
}

// shows a short message under the page title
var messageTimer = null;
function showMessage(text, isError) {
  var box = byId('message');
  box.textContent = text;
  box.className = isError ? 'error' : 'ok';
  clearTimeout(messageTimer);
  messageTimer = setTimeout(function () { box.textContent = ''; }, 6000);
}

// Runs an action (a function that talks to the server).
// The button is disabled while waiting, and an error is shown as a message.
async function run(button, action) {
  if (button) button.disabled = true;
  try {
    return await action();
  } catch (e) {
    showMessage(e.message, true);
  } finally {
    if (button) button.disabled = false;
  }
}

// organism type as coloured text, for example "Herbivore"
function typeText(type, name) {
  return '<span class="type' + type + '">' + esc(name || TYPE_NAMES[type]) + '</span>';
}

// shows text in a result box
function showResult(box, html, isError) {
  box.hidden = false;
  box.className = isError ? 'result error' : 'result';
  box.innerHTML = html;
}

// short explanation at the top of a module page
function aboutBox(cText, cppText, combinedText) {
  return '<div class="about">' +
           '<p><b>C (data structure):</b> ' + cText + '</p>' +
           '<p><b>C++ (OOP):</b> ' + cppText + '</p>' +
           '<p><b>Combined:</b> ' + combinedText + '</p>' +
         '</div>';
}

// ----- the application -----

var Pages = {};      // every page file adds one entry:  Pages.name = { title, render(container) }

var App = {
  current: 'dashboard',

  start() {
    var self = this;
    byId('login-form').addEventListener('submit', function (e) {
      e.preventDefault();
      self.login();
    });
    byId('logout-btn').addEventListener('click', function () { self.logout(); });
    window.addEventListener('hashchange', function () { self.route(); });

    if (Api.token) {
      this.showApp();
    } else {
      this.showLogin();
    }
  },

  async login() {
    var errorBox = byId('login-error');
    errorBox.hidden = true;
    try {
      var data = await Api.post('/api/login', {
        username: byId('login-user').value,
        password: byId('login-pass').value
      });
      Api.saveSession(data.token, data.username);
      byId('login-pass').value = '';
      this.showApp();
    } catch (e) {
      errorBox.textContent = e.message;
      errorBox.hidden = false;
    }
  },

  async logout() {
    try {
      await Api.post('/api/logout');
    } catch (e) {
      // nothing to do
    }
    Api.clearSession();
    location.hash = '';
    this.showLogin();
  },

  showLogin() {
    byId('app-view').hidden = true;
    byId('login-view').hidden = false;
    byId('login-user').focus();
  },

  showApp() {
    byId('login-view').hidden = true;
    byId('app-view').hidden = false;
    byId('user-name').textContent = Api.user;
    if (!location.hash) {
      location.hash = '#/dashboard';
    }
    this.route();
  },

  // find out which page is wanted from the address (#/population)
  route() {
    if (!Api.token) return;
    var name = location.hash.replace('#/', '');
    if (!Pages[name]) name = 'dashboard';
    this.current = name;
    byId('message').textContent = '';
    this.reload();
  },

  // draw the current page again
  async reload() {
    var page = Pages[this.current];

    var links = document.querySelectorAll('#nav a');
    for (var i = 0; i < links.length; i++) {
      if (links[i].dataset.page === this.current) {
        links[i].classList.add('active');
      } else {
        links[i].classList.remove('active');
      }
    }
    byId('page-title').textContent = page.title;

    var container = byId('content');
    try {
      await page.render(container);
    } catch (e) {
      container.innerHTML = '<p class="error">Error: ' + esc(e.message) + '</p>';
    }
    this.refreshStatus();
  },

  // the line under the menu: day, season, weather, health
  async refreshStatus() {
    try {
      var data = await Api.get('/api/dashboard');
      var s = data.state;
      var text = 'Day: ' + s.day +
                 ' | Season: ' + esc(s.season) +
                 ' | Weather: ' + esc(s.weather) +
                 ' | Health: ' + s.health.toFixed(0) + ' (' + esc(s.healthLabel) + ')';
      if (s.pendingEvents) {
        text += ' | Pending events: ' + s.pendingEvents;
      }
      byId('status-bar').innerHTML = text;
    } catch (e) {
      // the page itself shows the error
    }
  }
};
