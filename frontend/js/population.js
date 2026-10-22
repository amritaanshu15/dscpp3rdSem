/*
 * Population page
 *   Module 1: array + linear search (C)   and   Organism classes (C++)
 *   Module 2: linked list (C)             and   changing the populations
 */
// <option> list for the "Type" select box (Plant, Herbivore, Carnivore, Bird)
function typeOptionsHtml() {
  var html = '';
  for (var t = 0; t < TYPE_NAMES.length; t++) {
    html += '<option value="' + t + '">' + TYPE_NAMES[t] + '</option>';
  }
  return html;
}

Pages.population = {
  title: 'Population',

  async render(c) {
    var org = await Api.get('/api/population/organisms');
    var stats = await Api.get('/api/population/stats');
    var list = await Api.get('/api/population/list');

    // rows of the organism table
    var rows = '';
    for (var i = 0; i < org.organisms.length; i++) {
      var o = org.organisms[i];
      rows += '<tr><td>' + o.id + '</td><td>' + esc(o.name) + '</td><td>' + typeText(o.type, o.typeName) +
              '</td><td class="num">' + o.population + '</td><td>' + esc(o.diet) + '</td></tr>';
    }

    // rows of the statistics table
    var statRows = '';
    for (var i = 0; i < stats.byType.length; i++) {
      var t = stats.byType[i];
      statRows += '<tr><td>' + typeText(t.type, t.typeName) + '</td><td class="num">' + t.species + '</td><td class="num">' + t.population + '</td></tr>';
    }

    // the linked list drawn as boxes, and the options for the select boxes
    var nodes = '<span class="small">HEAD</span><span class="arrow">-&gt;</span>';
    var nodeOptions = '';
    for (var i = 0; i < list.nodes.length; i++) {
      var n = list.nodes[i];
      nodes += '<span class="node">' + esc(n.name) + ' (id ' + n.id + ', pop ' + n.population + ')</span><span class="arrow">-&gt;</span>';
      nodeOptions += '<option value="' + n.id + '">' + esc(n.name) + ' (id ' + n.id + ')</option>';
    }
    nodes += '<span class="small">NULL</span>';

    c.innerHTML =
      '<h3>Module 1: Organism and Population Management</h3>' +
      aboutBox('array with linear search. Add is O(1), search is O(n).',
               'class Organism with the child classes Plant, Animal, Herbivore, Carnivore and Bird (inheritance and overriding).',
               'The populations are copied into the C array. C finds the record, then the C++ object with the same id gives the diet and description.') +

      '<div class="box"><h3>Organism table</h3><div class="table-wrap"><table>' +
        '<tr><th>ID</th><th>Name</th><th>Type</th><th class="num">Population</th><th>Diet</th></tr>' + rows +
      '</table></div></div>' +

      '<div class="two-cols">' +
        '<div class="box"><h3>Search organism</h3>' +
          '<form id="search-form">' +
            '<div class="form-line"><label>Search by</label><select id="search-by"><option value="id">ID</option><option value="name">Name</option></select></div>' +
            '<div class="form-line"><label>Value</label><input id="search-value" placeholder="for example 3 or Deer" required></div>' +
            '<button type="submit">Search</button>' +
          '</form>' +
          '<div id="search-result" hidden></div>' +
        '</div>' +
        '<div class="box"><h3>Statistics</h3><table>' +
          '<tr><th>Type</th><th class="num">Species</th><th class="num">Individuals</th></tr>' + statRows +
          '<tr><td><b>Total</b></td><td class="num"><b>' + stats.totalSpecies + '</b></td><td class="num"><b>' + stats.totalPopulation + '</b></td></tr>' +
        '</table></div>' +
      '</div>' +

      '<h3>Module 2: Dynamic Organism Collection</h3>' +
      aboutBox('singly linked list. Every organism is a node made with malloc() and removed with free().',
               'classes Resource, Food, Water, Shelter and Environment (see the Resources page).',
               'The linked list holds the real populations. Inserting an organism also creates a C++ object, and deleting it frees both.') +

      '<div class="box"><h3>Linked list (' + list.count + ' nodes)</h3>' + nodes + '</div>' +

      '<div class="two-cols">' +
        '<div class="box"><h3>Insert organism</h3>' +
          '<form id="add-form">' +
            '<div class="form-line"><label>Name</label><input id="add-name" maxlength="31" required></div>' +
            '<div class="form-line"><label>Type</label><select id="add-type">' + typeOptionsHtml() + '</select></div>' +
            '<div class="form-line"><label>Population</label><input id="add-pop" type="number" min="0" max="100000" required></div>' +
            '<button type="submit">Insert</button>' +
          '</form></div>' +
        '<div class="box"><h3>Update population</h3>' +
          '<form id="upd-form">' +
            '<div class="form-line"><label>Organism</label><select id="upd-id">' + nodeOptions + '</select></div>' +
            '<div class="form-line"><label>New population</label><input id="upd-pop" type="number" min="0" max="100000" required></div>' +
            '<button type="submit">Update</button>' +
          '</form></div>' +
        '<div class="box"><h3>Delete organism</h3>' +
          '<form id="del-form">' +
            '<div class="form-line"><label>Organism</label><select id="del-id">' + nodeOptions + '</select></div>' +
            '<button type="submit">Delete</button>' +
          '</form></div>' +
      '</div>';

    this.bind(c);
  },

  bind(c) {
    var self = this;

    // search
    byId('search-form').addEventListener('submit', async function (e) {
      e.preventDefault();
      var by = byId('search-by').value;
      var value = byId('search-value').value.trim();
      var box = byId('search-result');
      try {
        var r = await Api.get('/api/population/search?' + by + '=' + encodeURIComponent(value));
        var o = r.organism;
        var text = '<b>' + esc(o.name) + '</b> (' + typeText(o.type, o.typeName) + ')<br>' +
                   'ID: ' + o.id + ', population: ' + o.population + '<br>' +
                   esc(o.description) + '<br>' +
                   'Diet: ' + esc(o.diet) + '. Appetite: ' + o.appetite + ' per day. ' +
                   'Birth rate: ' + o.birthRate + '. Death rate: ' + o.deathRate + '.';
        if (o.capacity) {
          text += ' Capacity: ' + o.capacity + '.';
        }
        text += '<br><span class="small">Linear search found it at index ' + r.index + ' after ' + r.comparisons + ' comparison(s).</span>';
        showResult(box, text, false);
      } catch (err) {
        showResult(box, esc(err.message), true);
      }
    });

    // insert, update and delete all work the same way:
    // send the request, show the server's message, draw the page again
    function onSubmit(formId, sendRequest) {
      byId(formId).addEventListener('submit', function (e) {
        e.preventDefault();
        run(e.submitter, async function () {
          var r = await sendRequest();
          showMessage(r.message);
          await self.render(c);
          App.refreshStatus();
        });
      });
    }

    onSubmit('add-form', function () {
      return Api.post('/api/population/organisms', {
        name: byId('add-name').value,
        type: byId('add-type').value,
        population: byId('add-pop').value
      });
    });
    onSubmit('upd-form', function () {
      return Api.put('/api/population/organisms/' + byId('upd-id').value, { population: byId('upd-pop').value });
    });
    onSubmit('del-form', function () {
      return Api.del('/api/population/organisms/' + byId('del-id').value);
    });
  }
};
