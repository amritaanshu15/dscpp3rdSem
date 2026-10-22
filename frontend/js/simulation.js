/*
 * Simulation page (Module 5)
 *   C sorting, binary search, hash table and statistics   and   C++ Ecosystem class
 */
Pages.simulation = {
  title: 'Simulation',

  async render(c) {
    var data = await Api.get('/api/dashboard');
    var s = data.state;
    var historyData = await Api.get('/api/simulation/history');
    var history = historyData.history;
    var marks = { markers: historyData.events };
    var an = await Api.get('/api/simulation/analysis');
    var st = an.stats;

    // numbers for the three charts
    var plants = [], herbivores = [], carnivores = [], birds = [], health = [];
    for (var i = 0; i < history.length; i++) {
      var h = history[i];
      plants.push({ x: h.day, y: h.plants });
      herbivores.push({ x: h.day, y: h.herbivores });
      carnivores.push({ x: h.day, y: h.carnivores });
      birds.push({ x: h.day, y: h.birds });
      health.push({ x: h.day, y: h.health });
    }

    // organisms sorted by population (bar chart)
    var popBars = [];
    for (var i = 0; i < an.byPopulation.length; i++) {
      var o = an.byPopulation[i];
      popBars.push({ label: o.name, value: o.population, color: TYPE_COLORS[TYPE_NAMES.indexOf(o.typeName)] });
    }

    // organisms sorted by name (table)
    var nameRows = '';
    for (var i = 0; i < an.byName.length; i++) {
      nameRows += '<tr><td>' + esc(an.byName[i].name) + '</td><td class="num">' + an.byName[i].population + '</td></tr>';
    }

    // hash table buckets
    var bucketRows = '';
    for (var i = 0; i < an.buckets.length; i++) {
      var b = an.buckets[i];
      var names = b.names.length ? esc(b.names.join(', ')) : '-';
      bucketRows += '<tr id="bucket-' + b.bucket + '"><td>' + b.bucket + '</td><td>' + names + '</td></tr>';
    }

    var largest = st.largest ? esc(st.largest) + ' / ' + esc(st.smallest) : '-';

    c.innerHTML =
      aboutBox('sorting (bubble sort by population, selection sort by name, both O(n*n)), binary search O(log n), and a hash table with chaining (average O(1)).',
               'class Ecosystem. It has the organisms, the Environment, Season, Weather and the FoodWeb, and it applies events. simulateDay() updates the resources, applies the rules, and runs the food-web interaction.',
               'Every day the C++ Ecosystem updates the organisms. The numbers are stored in the C linked list, events wait in the C queues, and the results are sorted and searched with C algorithms.') +

      '<p class="small">Educational simulation: simple rules. Random Heavy Rain or Drought events only happen when the switch below is on. It does not predict a real ecosystem.</p>' +

      Controls.html(s) +

      '<div class="box"><h3>Plants over time</h3>' +
        Charts.lines([{ name: 'Plants', color: TYPE_COLORS[0], points: plants }], marks) + '</div>' +
      '<div class="box"><h3>Animals over time</h3>' +
        Charts.lines([
          { name: 'Herbivores', color: TYPE_COLORS[1], points: herbivores },
          { name: 'Carnivores', color: TYPE_COLORS[2], points: carnivores },
          { name: 'Birds', color: TYPE_COLORS[3], points: birds }
        ], marks) + '</div>' +
      '<div class="box"><h3>Ecosystem health over time (now ' + s.health.toFixed(1) + ', ' + esc(s.healthLabel) + ')</h3>' +
        Charts.lines([{ name: 'Health', color: '#2a8a2a', points: health }], { yMax: 100, markers: historyData.events }) + '</div>' +

      '<h3>Analysis (C algorithms)</h3>' +
      '<div class="two-cols">' +
        '<div class="box"><h3>Sorted by population (bubble sort)</h3>' + Charts.bars(popBars) + '</div>' +
        '<div class="box"><h3>Sorted by name (selection sort)</h3><table>' +
          '<tr><th>Name</th><th class="num">Population</th></tr>' + nameRows + '</table></div>' +
      '</div>' +

      '<div class="box"><h3>Population statistics</h3><table>' +
        '<tr><td>Total individuals</td><td class="num">' + st.total + '</td><td>Average per species</td><td class="num">' + st.average + '</td></tr>' +
        '<tr><td>Median</td><td class="num">' + st.median + '</td><td>Largest / smallest</td><td class="num">' + largest + '</td></tr>' +
      '</table></div>' +

      '<div class="two-cols">' +
        '<div class="box"><h3>Hash table (' + an.hashSize + ' buckets, chaining)</h3><table>' +
          '<tr><th>Bucket</th><th>Organisms stored here</th></tr>' + bucketRows + '</table></div>' +
        '<div class="box"><h3>Search: hash, binary and linear</h3>' +
          '<form id="an-form">' +
            '<div class="form-line"><label>Organism name</label><input id="an-name" placeholder="for example Fox" required></div>' +
            '<button type="submit">Search</button>' +
          '</form>' +
          '<div id="an-result" hidden></div>' +
        '</div>' +
      '</div>' +

      '<div class="box"><h3>What-if comparison</h3>' +
        '<p class="small">Each scenario runs in its own temporary simulation, so your current simulation is not changed.</p>' +
        '<div class="form-line"><label>Days (1 to 200)</label><input id="cmp-days" type="number" min="1" max="200" value="50" style="width:80px"> ' +
        '<button id="cmp-run">Run all 4 scenarios</button></div>' +
        '<div id="cmp-result"></div>' +
      '</div>';

    var self = this;
    Controls.bind(c, function () { return self.render(c); });

    byId('an-form').addEventListener('submit', function (e) {
      e.preventDefault();
      self.search();
    });
    byId('cmp-run').addEventListener('click', function (e) {
      run(e.target, function () { return self.compare(); });
    });
  },

  // search for an organism with the three methods
  async search() {
    var box = byId('an-result');

    // remove the old yellow mark from the hash table
    var rows = document.querySelectorAll('tr.hit');
    for (var i = 0; i < rows.length; i++) {
      rows[i].classList.remove('hit');
    }

    try {
      var r = await Api.get('/api/simulation/search?name=' + encodeURIComponent(byId('an-name').value.trim()));
      var row = byId('bucket-' + r.hash.bucket);
      if (row) row.classList.add('hit');

      var found = r.hash.found;
      var text;
      if (found) {
        text = 'Found <b>' + esc(r.hash.organism.name) + '</b> (' + esc(r.hash.organism.typeName) + '), population ' + r.hash.organism.population + '.';
      } else {
        text = esc(r.name) + ' was not found.';
      }
      text += '<table style="margin-top:8px">' +
        '<tr><th>Method</th><th class="num">Comparisons</th><th>Idea</th></tr>' +
        '<tr><td>Hash table</td><td class="num">' + r.hash.comparisons + '</td><td>hash(name) = bucket ' + r.hash.bucket + ', only that chain is checked</td></tr>' +
        '<tr><td>Binary search</td><td class="num">' + r.binary.comparisons + '</td><td>names are sorted, the range is halved every step</td></tr>' +
        '<tr><td>Linear search</td><td class="num">' + r.linear.comparisons + '</td><td>every organism is checked one by one</td></tr>' +
        '</table>';
      showResult(box, text, !found);
    } catch (err) {
      showResult(box, esc(err.message), true);
    }
  },

  // run the four scenarios and show the result
  async compare() {
    var days = byId('cmp-days').value;
    var r = await Api.get('/api/simulation/compare?days=' + encodeURIComponent(days));
    var colors = { normal: '#2a8a2a', drought: '#e08a00', fire: '#cc2222', predator: '#2255cc' };

    var rows = '';
    var series = [];
    for (var i = 0; i < r.scenarios.length; i++) {
      var sc = r.scenarios[i];
      rows += '<tr><td><i style="display:inline-block;width:11px;height:11px;background:' + colors[sc.key] + '"></i> ' + esc(sc.label) + '</td>' +
              '<td class="num">' + sc.health.toFixed(1) + '</td><td class="num">' + sc.totalOrganisms + '</td>' +
              '<td class="num">' + sc.plants + '</td><td class="num">' + sc.herbivores + '</td>' +
              '<td class="num">' + sc.carnivores + '</td><td class="num">' + sc.birds + '</td>' +
              '<td class="num">' + sc.speciesAlive + '</td></tr>';

      var points = [];
      for (var k = 0; k < sc.series.length; k++) {
        points.push({ x: sc.series[k].day, y: sc.series[k].health });
      }
      series.push({ name: sc.label, color: colors[sc.key], points: points });
    }

    byId('cmp-result').innerHTML =
      '<div class="table-wrap" style="margin-top:10px"><table>' +
        '<tr><th>Scenario</th><th class="num">Health</th><th class="num">Total</th><th class="num">Plants</th><th class="num">Herbivores</th><th class="num">Carnivores</th><th class="num">Birds</th><th class="num">Species alive</th></tr>' +
        rows + '</table></div>' +
      '<p class="small">Result after ' + r.days + ' simulated days. The chart shows the ecosystem health (0 to 100) for every day.</p>' +
      Charts.lines(series, { yMax: 100 });
  }
};
