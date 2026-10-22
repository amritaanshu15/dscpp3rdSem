/*
 * Resources page (Module 2)
 *   C linked list (populations)  and  C++ Environment (Food, Water, Shelter)
 */
Pages.resources = {
  title: 'Resources',

  async render(c) {
    var r = await Api.get('/api/resources');
    var list = await Api.get('/api/population/list');

    // one table row for food, water and shelter
    function resourceRow(res) {
      var color = '#2a8a2a';
      if (res.percent < 50) color = '#e08a00';
      if (res.percent < 20) color = '#cc2222';
      return '<tr><td>' + esc(res.name) + '</td><td class="num">' + res.amount + '</td><td class="num">' + res.capacity + '</td>' +
             '<td><div class="small-bar"><div style="width:' + res.percent + '%; background:' + color + '"></div></div> ' + res.percent + '%</td>' +
             '<td>' + esc(res.status) + '</td><td>' + esc(res.description) + '</td></tr>';
    }

    // the three rules of the simulation. A rule is "active" when the engine says so.
    var rules = [
      ['Rule 1', 'Food below 30%: herbivores lose 5% of their population every day'],
      ['Rule 2', 'Water below 20%: animals lose 3% and plants lose 2% every day'],
      ['Rule 3', 'Shelter below 30%: herbivores and birds lose 2% every day']
    ];
    var ruleRows = '';
    for (var i = 0; i < rules.length; i++) {
      var active = false;
      for (var k = 0; k < r.activeEffects.length; k++) {
        if (r.activeEffects[k].indexOf(rules[i][0] + ' active') === 0) active = true;
      }
      ruleRows += '<tr class="' + (active ? 'active' : '') + '"><td>' + rules[i][0] + '</td><td>' + rules[i][1] + '</td><td>' + (active ? 'ACTIVE' : 'not active') + '</td></tr>';
    }

    // herbivores for the bar chart
    var herbivoreBars = [];
    for (var i = 0; i < r.herbivores.length; i++) {
      herbivoreBars.push({ label: r.herbivores[i].name, value: r.herbivores[i].population, color: TYPE_COLORS[1] });
    }

    // plants for the experiment
    var plantOptions = '';
    for (var i = 0; i < list.nodes.length; i++) {
      if (list.nodes[i].type === 0) {
        plantOptions += '<option value="' + list.nodes[i].id + '">' + esc(list.nodes[i].name) + ' (now ' + list.nodes[i].population + ')</option>';
      }
    }

    var modifierText = 'x' + r.growthModifier;
    if (r.modifierDaysLeft) {
      modifierText += ' (' + r.modifierDaysLeft + ' more days)';
    }

    c.innerHTML =
      aboutBox('linked list. It holds the current population of every organism, and the numbers change every simulated day.',
               'class Resource with the child classes Food, Water and Shelter. Environment contains one of each (composition).',
               'The C++ Environment measures food, water and shelter. Three simple rules use these values to change the populations in the C linked list. Food means the total plant population.') +

      '<div class="box"><h3>Food, water and shelter</h3><div class="table-wrap"><table>' +
        '<tr><th>Resource</th><th class="num">Amount</th><th class="num">Capacity</th><th>Level</th><th>Status</th><th>Description</th></tr>' +
        resourceRow(r.food) + resourceRow(r.water) + resourceRow(r.shelter) +
      '</table></div></div>' +

      '<div class="box"><h3>Effect of resources on organisms</h3><table>' +
        '<tr><th>Rule</th><th>What happens</th><th>Now</th></tr>' + ruleRows +
      '</table>' +
      '<p class="small">Season: ' + esc(r.season) + '. Weather: ' + esc(r.weather) + '. Plant growth multiplier: ' + modifierText + '.</p></div>' +

      '<div class="two-cols">' +
        '<div class="box"><h3>Herbivore populations</h3>' + Charts.bars(herbivoreBars) + '</div>' +
        '<div class="box"><h3>Experiment: reduce the food</h3>' +
          '<p class="small">Lower a plant population, then advance 10 days and watch the herbivores.</p>' +
          '<div class="form-line"><label>Plant</label><select id="exp-plant">' + plantOptions + '</select></div>' +
          '<div class="form-line"><label>New population</label><input id="exp-pop" type="number" min="0" value="20" style="width:90px"></div>' +
          '<button id="exp-set">Set population</button> <button id="exp-step">Advance 10 days</button>' +
        '</div>' +
      '</div>';

    var self = this;
    byId('exp-set').addEventListener('click', function (e) {
      run(e.target, async function () {
        var res = await Api.put('/api/population/organisms/' + byId('exp-plant').value, { population: byId('exp-pop').value });
        showMessage(res.message);
        await self.render(c);
        App.refreshStatus();
      });
    });
    byId('exp-step').addEventListener('click', function (e) {
      run(e.target, async function () {
        await Api.post('/api/simulation/step', { days: 10 });
        showMessage('Advanced 10 days');
        await self.render(c);
        App.refreshStatus();
      });
    });
  }
};
