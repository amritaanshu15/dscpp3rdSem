/*
 * controls.js - buttons to run the simulation (used on the Dashboard and Simulation pages)
 *   Start, Advance 1 / 10 / 100 days, the four scenarios, the random-events switch,
 *   and the banner that shows which event is active
 */
var Controls = {

  // Yellow / blue strip: which weather or event effect is active right now.
  // s = the "state" object from /api/dashboard
  banner(s) {
    var weatherActive = (s.weather !== 'Clear');
    if (!weatherActive && !s.modifierDaysLeft) return '';

    var text = '<b>Active now:</b> ' + esc(s.weather) + ' weather';
    if (s.modifierDaysLeft) {
      text += '. Plant growth is x' + s.growthModifier + ' for ' + s.modifierDaysLeft + ' more day(s)';
    }
    if (s.lastEvent) {
      text += '.<br><span class="small">Last event: ' + esc(s.lastEvent.name) + ' on day ' + s.lastEvent.day +
              ' (' + esc(s.lastEvent.effect) + ')</span>';
    }
    return '<div class="banner' + (s.weather === 'Rainy' ? ' rainy' : '') + '">' + text + '</div>';
  },

  // s = the "state" object (needed for the banner and for the switch)
  html(s) {
    return this.banner(s) + '<div class="box" id="sim-controls">' +
      '<h3>Simulation controls</h3>' +
      '<div class="buttons">' +
        '<button data-act="start">Start / Reset</button>' +
        '<button data-act="step" data-days="1">Advance 1 day</button>' +
        '<button data-act="step" data-days="10">Advance 10 days</button>' +
        '<button data-act="step" data-days="100">Advance 100 days</button>' +
      '</div>' +
      '<p class="small">Scenarios (each one starts the ecosystem again from day 0):</p>' +
      '<div class="buttons">' +
        '<button data-act="scenario" data-name="normal">Normal</button>' +
        '<button data-act="scenario" data-name="drought">Drought</button>' +
        '<button data-act="scenario" data-name="fire">Forest fire</button>' +
        '<button data-act="scenario" data-name="predator">Predator removal</button>' +
      '</div>' +
      '<p><label><input type="checkbox" id="random-toggle"' + (s.randomEvents ? ' checked' : '') + '> ' +
        'Random events (each day a 3% chance of Heavy Rain or Drought)</label></p>' +
      '<p class="small" id="sim-msg"></p>' +
    '</div>';
  },

  // adds the click handlers. onDone() is called after every successful click
  bind(root, onDone) {
    byId('random-toggle').addEventListener('change', function (e) {
      run(e.target, async function () {
        var data = await Api.post('/api/simulation/random-events', { enabled: e.target.checked });
        showMessage(data.message);
      });
    });

    var buttons = root.querySelectorAll('#sim-controls button');
    for (var i = 0; i < buttons.length; i++) {
      buttons[i].addEventListener('click', function (e) {
        var button = e.currentTarget;
        run(button, async function () {
          var data;
          if (button.dataset.act === 'start') {
            data = await Api.post('/api/simulation/start');
          } else if (button.dataset.act === 'step') {
            data = await Api.post('/api/simulation/step', { days: Number(button.dataset.days) });
          } else {
            data = await Api.post('/api/simulation/scenario', { name: button.dataset.name });
          }
          if (data.message) {
            showMessage(data.message);
          }
          await onDone(data);
          App.refreshStatus();
          if (button.dataset.act === 'step') {
            var msg = byId('sim-msg');
            if (msg) {
              var text = 'Advanced ' + data.daysAdvanced + ' day(s). Events processed: ' + data.eventsProcessed + '.';
              for (var i = 0; i < data.events.length; i++) {
                text += ' ' + data.events[i].name + ' on day ' + data.events[i].day + '.';
              }
              msg.textContent = text;
            }
          }
        });
      });
    }
  }
};
