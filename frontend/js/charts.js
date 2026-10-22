/*
 * charts.js - two small chart functions (no libraries).
 * Both return a piece of HTML as text.
 */
var TYPE_COLORS = ['#2a8a2a', '#e08a00', '#cc2222', '#2255cc'];   // plant, herbivore, carnivore, bird
var TYPE_NAMES = ['Plant', 'Herbivore', 'Carnivore', 'Bird'];

var Charts = {

  // horizontal bars.   items = [ { label, value, color } ]
  bars(items) {
    if (items.length === 0) {
      return '<p class="small">No data.</p>';
    }
    var max = 1;
    for (var i = 0; i < items.length; i++) {
      if (items[i].value > max) max = items[i].value;
    }

    var html = '';
    for (var i = 0; i < items.length; i++) {
      var item = items[i];
      var percent = item.value / max * 100;
      html += '<div class="bar-row">' +
                '<div class="bar-label">' + esc(item.label) + '</div>' +
                '<div class="bar-track"><div class="bar-fill" style="width:' + percent.toFixed(1) + '%; background:' + item.color + '"></div></div>' +
                '<div class="bar-value">' + item.value + '</div>' +
              '</div>';
    }
    return html;
  },

  // line chart.   series = [ { name, color, points: [ {x, y} ] } ]
  lines(series, options) {
    options = options || {};
    var W = 560, H = 230;                      // size of the drawing
    var left = 45, right = 12, top = 12, bottom = 28;

    // collect all points to find the biggest and smallest values
    var all = [];
    for (var i = 0; i < series.length; i++) {
      all = all.concat(series[i].points);
    }
    if (all.length < 2) {
      return '<p class="small">Advance the simulation to see this chart.</p>';
    }

    var xMin = all[0].x, xMax = all[0].x, yMax = all[0].y;
    for (var i = 0; i < all.length; i++) {
      if (all[i].x < xMin) xMin = all[i].x;
      if (all[i].x > xMax) xMax = all[i].x;
      if (all[i].y > yMax) yMax = all[i].y;
    }
    if (options.yMax) {
      yMax = options.yMax;
    } else {
      yMax = yMax * 1.1;
    }
    if (yMax <= 0) yMax = 1;

    // turn a value into a position on the drawing
    function posX(x) {
      if (xMax === xMin) return left;
      return left + (x - xMin) / (xMax - xMin) * (W - left - right);
    }
    function posY(y) {
      return top + (1 - y / yMax) * (H - top - bottom);
    }

    var svg = '';

    // grid lines and numbers on the y axis
    for (var i = 0; i <= 4; i++) {
      var value = yMax * i / 4;
      var y = posY(value);
      svg += '<line x1="' + left + '" x2="' + (W - right) + '" y1="' + y + '" y2="' + y + '" stroke="#ddd"/>';
      svg += '<text x="' + (left - 5) + '" y="' + (y + 4) + '" text-anchor="end" font-size="12" fill="#444">' + Math.round(value) + '</text>';
    }
    // numbers on the x axis (days)
    for (var i = 0; i <= 4; i++) {
      var day = xMin + (xMax - xMin) * i / 4;
      svg += '<text x="' + posX(day) + '" y="' + (H - 8) + '" text-anchor="middle" font-size="12" fill="#444">' + Math.round(day) + '</text>';
    }
    // the two axes
    svg += '<line x1="' + left + '" x2="' + left + '" y1="' + top + '" y2="' + (H - bottom) + '" stroke="#000"/>';
    svg += '<line x1="' + left + '" x2="' + (W - right) + '" y1="' + (H - bottom) + '" y2="' + (H - bottom) + '" stroke="#000"/>';

    // a dashed red line at the day of every event (options.markers = [ {day, name} ])
    // The name is written above the line. Labels that would overlap an earlier one
    // move to a lower row; if all 3 rows are taken, only the line is drawn.
    var shortName = { 'Heavy Rain': 'Rain', 'Forest Fire': 'Fire', 'Predator Removal': 'No predators', 'Seasonal Change': 'Season' };
    var markers = options.markers || [];
    var rowEnd = [-1, -1, -1];                 // x position where the last label of each row ends
    var shown = 0;
    for (var m = 0; m < markers.length; m++) {
      if (markers[m].day < xMin || markers[m].day > xMax) continue;
      var mx = posX(markers[m].day);
      svg += '<line x1="' + mx + '" x2="' + mx + '" y1="' + top + '" y2="' + (H - bottom) + '" stroke="#b00000" stroke-dasharray="4 3"/>';
      shown++;

      var label = shortName[markers[m].name] || markers[m].name;
      var labelWidth = label.length * 6 + 6;
      var atRightEdge = (mx + labelWidth > W - right);      // not enough room on the right side
      var from = atRightEdge ? mx - labelWidth : mx;
      for (var row = 0; row < rowEnd.length; row++) {
        if (from > rowEnd[row]) {
          svg += '<text x="' + (atRightEdge ? mx - 3 : mx + 3) + '" y="' + (top + 10 + row * 12) + '" font-size="11" fill="#b00000"' +
                 (atRightEdge ? ' text-anchor="end"' : '') + '>' + esc(label) + '</text>';
          rowEnd[row] = atRightEdge ? mx : mx + labelWidth;
          break;
        }
      }
    }

    // one line for every series
    for (var s = 0; s < series.length; s++) {
      var pointsText = '';
      for (var p = 0; p < series[s].points.length; p++) {
        pointsText += posX(series[s].points[p].x).toFixed(1) + ',' + posY(series[s].points[p].y).toFixed(1) + ' ';
      }
      svg += '<polyline fill="none" stroke="' + series[s].color + '" stroke-width="2" points="' + pointsText + '"/>';
    }

    var legend = '';
    if (series.length > 1) {
      legend = '<div class="legend">';
      for (var s = 0; s < series.length; s++) {
        legend += '<span><i style="background:' + series[s].color + '"></i>' + esc(series[s].name) + '</span>';
      }
      legend += '</div>';
    }

    return '<div class="chart"><svg viewBox="0 0 ' + W + ' ' + H + '">' + svg + '</svg>' +
           '<div class="small">x axis: day' + (shown ? '. Dashed red lines = events' : '') + '</div>' + legend + '</div>';
  }
};
