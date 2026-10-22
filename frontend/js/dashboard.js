/* Dashboard page: numbers, simulation buttons and two charts */
Pages.dashboard = {
  title: 'Dashboard',

  async render(container) {
    var data = await Api.get('/api/dashboard');
    var s = data.state;
    var historyData = await Api.get('/api/simulation/history');
    var history = historyData.history;

    // data for the health chart
    var healthPoints = [];
    for (var i = 0; i < history.length; i++) {
      healthPoints.push({ x: history[i].day, y: history[i].health });
    }

    // data for the bar chart
    var bars = [];
    for (var i = 0; i < s.species.length; i++) {
      bars.push({ label: s.species[i].name, value: s.species[i].population, color: TYPE_COLORS[s.species[i].type] });
    }

    var legend = '<div class="legend">';
    for (var i = 0; i < TYPE_NAMES.length; i++) {
      legend += '<span><i style="background:' + TYPE_COLORS[i] + '"></i>' + TYPE_NAMES[i] + '</span>';
    }
    legend += '</div>';

    container.innerHTML =
      '<p class="small">EcoLife is an educational simulation. It uses simple rules (random events only when you switch them on), ' +
      'so it does not predict a real ecosystem.</p>' +

      '<div class="box"><h3>Current status</h3>' +
      '<table>' +
        '<tr><td>Day</td><td>' + s.day + '</td><td>Season</td><td>' + esc(s.season) + '</td></tr>' +
        '<tr><td>Weather</td><td>' + esc(s.weather) + '</td><td>Ecosystem health</td><td>' + s.health.toFixed(1) + ' (' + esc(s.healthLabel) + ')</td></tr>' +
        '<tr><td>Total organisms</td><td>' + s.totalOrganisms + ' (' + s.speciesCount + ' species)</td><td>Plants</td><td>' + s.plants + '</td></tr>' +
        '<tr><td>Herbivores</td><td>' + s.herbivores + '</td><td>Carnivores</td><td>' + s.carnivores + '</td></tr>' +
        '<tr><td>Birds</td><td>' + s.birds + '</td><td>Food</td><td>' + s.food + ' of ' + s.foodCapacity + '</td></tr>' +
        '<tr><td>Water</td><td>' + s.water + ' of ' + s.waterCapacity + '</td><td>Pending events</td><td>' + s.pendingEvents + '</td></tr>' +
      '</table></div>' +

      Controls.html(s) +

      '<div class="box"><h3>Population of each species</h3>' + Charts.bars(bars) + legend + '</div>' +

      '<div class="box"><h3>Ecosystem health over time</h3>' +
        Charts.lines([{ name: 'Health', color: '#2a8a2a', points: healthPoints }], { yMax: 100, markers: historyData.events }) + '</div>';

    Controls.bind(container, function () { return Pages.dashboard.render(container); });
  }
};
