/*
 * Food Web page (Module 4)
 *   C graph (adjacency matrix, DFS, BFS)   and   C++ FoodWeb, Predator and Prey classes
 */
Pages.foodweb = {
  title: 'Food Web',
  web: null,
  highlight: null,      // after DFS / BFS: { organismId: stepNumber }

  async render(c) {
    this.web = await Api.get('/api/foodweb');
    this.highlight = null;
    var w = this.web;

    // select boxes. Only animals can be an "eater" (plants eat nothing).
    var allOptions = '';
    var eaterOptions = '';
    for (var i = 0; i < w.nodes.length; i++) {
      var option = '<option value="' + w.nodes[i].id + '">' + esc(w.nodes[i].name) + '</option>';
      allOptions += option;
      if (w.nodes[i].type !== 0) eaterOptions += option;
    }

    var edgeRows = '';
    for (var i = 0; i < w.edges.length; i++) {
      var e = w.edges[i];
      edgeRows += '<tr><td>' + esc(e.text) + '</td><td><button data-food="' + e.food + '" data-eater="' + e.eater + '">Remove</button></td></tr>';
    }
    var edgeTable = '<p class="small">No relationships.</p>';
    if (w.edges.length > 0) {
      edgeTable = '<table>' + edgeRows + '</table>';
    }

    var roleRows = '';
    for (var i = 0; i < w.nodes.length; i++) {
      var n = w.nodes[i];
      roleRows += '<tr><td>' + esc(n.name) + '</td><td>' + typeText(n.type, n.typeName) + '</td><td>' + esc(n.role) + '</td></tr>';
    }

    c.innerHTML =
      aboutBox('graph stored as an adjacency matrix. Every organism is a node and "A is eaten by B" is an arrow A -> B. DFS goes deep first (recursion), BFS goes level by level (queue). Both are O(V*V) with a matrix.',
               'classes FoodWeb, FeedingRelationship, Predator and Prey. The relationships are objects, and every member is a Predator or a Prey object.',
               'The C++ FoodWeb keeps the relationships. For a traversal they are loaded into the C graph, DFS or BFS runs in C, and the node numbers come back as names.') +

      '<div class="box"><h3>Food web diagram (arrow means "is eaten by")</h3>' +
        '<div class="web-diagram" id="web-diagram"></div></div>' +

      '<div class="two-cols">' +
        '<div class="box"><h3>Traversal</h3>' +
          '<div class="form-line"><label>Start at</label><select id="fw-start">' + allOptions + '</select></div>' +
          '<button id="fw-dfs">DFS</button> <button id="fw-bfs">BFS</button> <button id="fw-comp">Connected organisms</button>' +
          '<div id="fw-result" hidden></div></div>' +
        '<div class="box"><h3>Add a feeding relationship</h3>' +
          '<div class="form-line"><label>Food (is eaten)</label><select id="fw-food">' + allOptions + '</select></div>' +
          '<div class="form-line"><label>Eater</label><select id="fw-eater">' + eaterOptions + '</select></div>' +
          '<button id="fw-add">Add</button></div>' +
      '</div>' +

      '<div class="two-cols">' +
        '<div class="box"><h3>Relationships (' + w.edges.length + ')</h3>' + edgeTable + '</div>' +
        '<div class="box"><h3>Roles (C++ Predator / Prey objects)</h3><table>' +
          '<tr><th>Organism</th><th>Type</th><th>Role</th></tr>' + roleRows + '</table></div>' +
      '</div>';

    this.draw();
    this.bind(c);
  },

  // draws the diagram with SVG
  draw() {
    var nodes = this.web.nodes;
    var edges = this.web.edges;
    var diagram = byId('web-diagram');

    if (nodes.length === 0) {
      diagram.innerHTML = '<p class="small">There are no organisms.</p>';
      return;
    }

    // The column of an organism = length of the longest food chain below it.
    // Plants are in column 0, animals that eat plants in column 1, and so on.
    var level = {};
    for (var i = 0; i < nodes.length; i++) {
      level[nodes[i].id] = 0;
    }
    for (var pass = 0; pass < nodes.length; pass++) {
      var changed = false;
      for (var i = 0; i < edges.length; i++) {
        var newLevel = level[edges[i].food] + 1;
        if (level[edges[i].eater] < newLevel && newLevel < nodes.length) {
          level[edges[i].eater] = newLevel;
          changed = true;
        }
      }
      if (!changed) break;
    }

    // put the organisms of every column below each other
    var columns = [];
    for (var i = 0; i < nodes.length; i++) {
      var col = level[nodes[i].id];
      if (!columns[col]) columns[col] = [];
      columns[col].push(nodes[i]);
    }
    var maxRows = 1;
    for (var i = 0; i < columns.length; i++) {
      if (columns[i] && columns[i].length > maxRows) maxRows = columns[i].length;
    }

    var boxW = 130, boxH = 44, gapX = 90, gapY = 20, margin = 15;
    var width = margin * 2 + columns.length * boxW + (columns.length - 1) * gapX;
    var height = margin * 2 + maxRows * boxH + (maxRows - 1) * gapY;

    var pos = {};
    for (var col = 0; col < columns.length; col++) {
      if (!columns[col]) continue;
      var startY = margin + (maxRows - columns[col].length) * (boxH + gapY) / 2;
      for (var row = 0; row < columns[col].length; row++) {
        pos[columns[col][row].id] = {
          x: margin + col * (boxW + gapX),
          y: startY + row * (boxH + gapY)
        };
      }
    }

    var fills = ['#c8e6c8', '#ffe0a8', '#f5b8b8', '#b8cff5'];     // light colour for every type
    var hl = this.highlight;
    var svg = '<defs><marker id="arrowhead" viewBox="0 0 10 10" refX="10" refY="5" markerWidth="7" markerHeight="7" orient="auto">' +
              '<path d="M0,0 L10,5 L0,10 z" fill="#333"/></marker></defs>';

    // arrows (from the food to the eater)
    for (var i = 0; i < edges.length; i++) {
      var from = pos[edges[i].food];
      var to = pos[edges[i].eater];
      if (!from || !to) continue;
      var lit = hl && hl[edges[i].food] && hl[edges[i].eater];
      svg += '<line x1="' + (from.x + boxW) + '" y1="' + (from.y + boxH / 2) + '" x2="' + (to.x - 2) + '" y2="' + (to.y + boxH / 2) + '"' +
             ' stroke="' + (lit ? '#000' : '#777') + '" stroke-width="' + (lit ? 2.5 : 1.2) + '" marker-end="url(#arrowhead)"/>';
    }

    // boxes
    for (var i = 0; i < nodes.length; i++) {
      var n = nodes[i];
      var p = pos[n.id];
      var step = hl ? hl[n.id] : 0;
      var opacity = (hl && !step) ? 0.35 : 1;
      var label = step ? step + '. ' + n.name : n.name;
      svg += '<g opacity="' + opacity + '">' +
             '<rect x="' + p.x + '" y="' + p.y + '" width="' + boxW + '" height="' + boxH + '" fill="' + fills[n.type] + '" stroke="#000" stroke-width="' + (step ? 3 : 1) + '"/>' +
             '<text x="' + (p.x + boxW / 2) + '" y="' + (p.y + 19) + '" text-anchor="middle" font-size="14" font-weight="bold" fill="#000">' + esc(label) + '</text>' +
             '<text x="' + (p.x + boxW / 2) + '" y="' + (p.y + 35) + '" text-anchor="middle" font-size="12" fill="#000">pop ' + n.population + '</text>' +
             '</g>';
    }

    diagram.innerHTML = '<svg width="' + width + '" height="' + height + '" viewBox="0 0 ' + width + ' ' + height + '">' + svg + '</svg>';
  },

  bind(c) {
    var self = this;
    var box = byId('fw-result');

    // DFS or BFS: kind is 'dfs' or 'bfs'
    async function traverse(kind, title, explanation) {
      var r = await Api.get('/api/foodweb/' + kind + '/' + byId('fw-start').value);
      self.highlight = {};
      var steps = '';
      for (var i = 0; i < r.order.length; i++) {
        self.highlight[r.order[i].id] = r.order[i].step;
        steps += '<span>' + r.order[i].step + '. ' + esc(r.order[i].name) + '</span>';
      }
      self.draw();
      showResult(box, '<b>' + title + '</b> from ' + esc(r.order[0].name) + ' visited ' + r.count + ' organism(s):' +
                      '<div class="steps">' + steps + '</div><p class="small">' + explanation + '</p>', false);
    }

    byId('fw-dfs').addEventListener('click', function (e) {
      run(e.target, function () {
        return traverse('dfs', 'Depth-first search', 'DFS follows one food chain as deep as possible before it goes back.');
      });
    });
    byId('fw-bfs').addEventListener('click', function (e) {
      run(e.target, function () {
        return traverse('bfs', 'Breadth-first search', 'BFS visits all organisms at distance 1, then distance 2, and so on.');
      });
    });

    byId('fw-comp').addEventListener('click', function (e) {
      run(e.target, async function () {
        var r = await Api.get('/api/foodweb/components');
        self.highlight = null;
        self.draw();
        var html = '<b>' + r.groupCount + ' connected group(s)</b> (direction of the arrows is ignored)';
        for (var g = 0; g < r.groups.length; g++) {
          var members = '';
          for (var m = 0; m < r.groups[g].members.length; m++) {
            members += '<span>' + esc(r.groups[g].members[m].name) + '</span>';
          }
          html += '<div class="steps">Group ' + r.groups[g].group + ': ' + members + '</div>';
        }
        showResult(box, html, false);
      });
    });

    byId('fw-add').addEventListener('click', function (e) {
      run(e.target, async function () {
        var r = await Api.post('/api/foodweb/relationships', { food: byId('fw-food').value, eater: byId('fw-eater').value });
        showMessage(r.message);
        await self.render(c);
      });
    });

    var removeButtons = c.querySelectorAll('button[data-food]');
    for (var i = 0; i < removeButtons.length; i++) {
      removeButtons[i].addEventListener('click', function (e) {
        var button = e.currentTarget;
        run(button, async function () {
          var r = await Api.del('/api/foodweb/relationships?food=' + button.dataset.food + '&eater=' + button.dataset.eater);
          showMessage(r.message);
          await self.render(c);
        });
      });
    }
  }
};
