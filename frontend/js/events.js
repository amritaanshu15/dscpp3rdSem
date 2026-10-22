/*
 * Events page (Module 3)
 *   C queue, priority queue and history array   and   C++ Event classes (Drought, ForestFire, ...)
 */
Pages.events = {
  title: 'Events',

  async render(c) {
    var ev = await Api.get('/api/events');

    // shows one queue as a numbered list. The first item is the one that is processed next.
    function queueList(items, showPriority) {
      if (items.length === 0) {
        return '<ol class="queue-list"><li class="small">empty</li></ol>';
      }
      var html = '<ol class="queue-list">';
      for (var i = 0; i < items.length; i++) {
        var e = items[i];
        html += '<li>' + (i === 0 ? '<b>' : '') + '#' + e.id + ' ' + esc(e.name) + (i === 0 ? ' (next)</b>' : '');
        html += ' <span class="small">' + (showPriority ? 'priority ' + e.priority + ', ' : '') + 'added on day ' + e.day + '</span></li>';
      }
      return html + '</ol>';
    }

    var typeOptions = '';
    for (var i = 0; i < ev.types.length; i++) {
      typeOptions += '<option value="' + ev.types[i].type + '">' + esc(ev.types[i].name) + ' (priority ' + ev.types[i].priority + ')</option>';
    }

    var historyRows = '';
    for (var i = 0; i < ev.history.length; i++) {
      var h = ev.history[i];
      historyRows += '<tr><td>' + h.id + '</td><td>' + esc(h.name) + '</td><td class="num">' + h.priority + '</td>' +
                     '<td class="num">' + h.addedDay + '</td><td class="num">' + h.processedDay + '</td><td>' + esc(h.effect) + '</td></tr>';
    }
    var historyTable = '<p class="small">No event has been processed yet.</p>';
    if (ev.history.length > 0) {
      historyTable = '<div class="table-wrap"><table>' +
        '<tr><th>#</th><th>Event</th><th class="num">Priority</th><th class="num">Added (day)</th><th class="num">Processed (day)</th><th>Effect</th></tr>' +
        historyRows + '</table></div>';
    }

    c.innerHTML =
      aboutBox('queue and priority queue. The FIFO queue is a circular array (enqueue and dequeue are O(1)). The priority queue is a binary heap (push and pop are O(log n)). Processed events are saved in a history array.',
               'class Event with the child classes Drought, ForestFire, PredatorRemoval, HeavyRain and SeasonalChange. Each child overrides the virtual function apply() (polymorphism).',
               'The C structures only store small event records. To process one, a C++ Event object is made, apply() changes the ecosystem, and the result is written to the C history.') +

      '<div class="two-cols">' +
        '<div class="box"><h3>Create an event</h3>' +
          '<div class="form-line"><label>Event type</label><select id="ev-type">' + typeOptions + '</select></div>' +
          '<div class="form-line"><label>Add to</label><select id="ev-queue"><option value="priority">Priority queue</option><option value="fifo">FIFO queue</option></select></div>' +
          '<button id="ev-add">Add event</button>' +
          '<p class="small" id="ev-desc"></p>' +
        '</div>' +
        '<div class="box"><h3>Compare FIFO and priority queue</h3>' +
          '<p class="small">The button adds the same 3 events to both queues in this order: Seasonal Change, Heavy Rain, Forest Fire. ' +
          'Then use "Process next" on each queue. FIFO takes Seasonal Change first, the priority queue takes Forest Fire first.</p>' +
          '<button id="ev-demo">Load demo events</button> <button id="ev-all">Process all pending</button>' +
        '</div>' +
      '</div>' +

      '<div class="two-cols">' +
        '<div class="box"><h3>FIFO queue</h3>' + queueList(ev.fifo, false) +
          '<p><button id="ev-next-fifo">Process next</button></p></div>' +
        '<div class="box"><h3>Priority queue</h3>' + queueList(ev.priorityQueue, true) +
          '<p><button id="ev-next-pq">Process next</button></p></div>' +
      '</div>' +

      '<div class="box"><h3>Event history (newest first)</h3>' + historyTable + '</div>';

    var self = this;
    var typeSelect = byId('ev-type');

    // show the description of the selected event type
    function showDescription() {
      var text = '';
      for (var i = 0; i < ev.types.length; i++) {
        if (String(ev.types[i].type) === typeSelect.value) text = ev.types[i].description;
      }
      byId('ev-desc').textContent = text;
    }
    typeSelect.addEventListener('change', showDescription);
    showDescription();

    // draw the page again after something changed
    async function done(message) {
      if (message) showMessage(message);
      await self.render(c);
      App.refreshStatus();
    }

    byId('ev-add').addEventListener('click', function (e) {
      run(e.target, async function () {
        var r = await Api.post('/api/events', { type: typeSelect.value, queue: byId('ev-queue').value });
        await done(r.message);
      });
    });

    byId('ev-demo').addEventListener('click', function (e) {
      run(e.target, async function () {
        var demoTypes = [5, 4, 2];       // Seasonal Change, Heavy Rain, Forest Fire
        for (var i = 0; i < demoTypes.length; i++) {
          await Api.post('/api/events', { type: demoTypes[i], queue: 'fifo' });
          await Api.post('/api/events', { type: demoTypes[i], queue: 'priority' });
        }
        await done('Demo events added to both queues');
      });
    });

    byId('ev-next-fifo').addEventListener('click', function (e) {
      run(e.target, async function () {
        var r = await Api.post('/api/events/process', { queue: 'fifo' });
        await done(r.processed.name + ': ' + r.effect);
      });
    });

    byId('ev-next-pq').addEventListener('click', function (e) {
      run(e.target, async function () {
        var r = await Api.post('/api/events/process', { queue: 'priority' });
        await done(r.processed.name + ': ' + r.effect);
      });
    });

    byId('ev-all').addEventListener('click', function (e) {
      run(e.target, async function () {
        var r = await Api.post('/api/events/process-all');
        await done(r.processedCount + ' event(s) processed');
      });
    });
  }
};
