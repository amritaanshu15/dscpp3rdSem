// draw.js - puts the game state (JSON from C++) on the screen.
// The browser does NOT decide anything, it only draws.

// map letter -> css class
var tileClass = {
    ".": "ground",
    "g": "grass",
    "w": "water",
    "b": "bush",
    "r": "rock",
    "x": "burnt"
};

// small picture on some tiles
var tileEmoji = {
    "b": "🌳",
    "r": "🪨",
    "w": "",
    "g": "",
    ".": "",
    "x": ""
};

// animal letter -> emoji (same letters as letter() in Animal.h)
var animalEmoji = {
    "F": "🦊",
    "E": "🦅",
    "D": "🦌",
    "T": "🐅",
    "L": "🐆",
    "S": "🐍",
    "C": "🐊"
};

// the animal you play
var playerEmoji = {
    "R": "🐇",
    "Q": "🐿️",
    "U": "🐢",
    "H": "🦔"
};

function drawBoard(state) {
    var board = document.getElementById("board");
    var html = "";

    for (var r = 0; r < state.map.length; r++) {
        var row = state.map[r];
        for (var c = 0; c < row.length; c++) {
            var letter = row[c];
            var cls = "cell " + tileClass[letter];
            var picture = tileEmoji[letter];
            var title = "";

            // is there an animal on this cell?
            for (var i = 0; i < state.animals.length; i++) {
                var a = state.animals[i];
                if (a.r == r && a.c == c) {
                    picture = animalEmoji[a.type];
                    title = a.name + ": " + a.info;
                }
            }

            // the player is drawn last so it is on top
            if (state.player.r == r && state.player.c == c) {
                picture = playerEmoji[state.player.kind];
                cls = cls + " me";
                title = "You (" + state.player.animal + ")";
            }

            html += '<div class="' + cls + '" title="' + title + '">' + picture + "</div>";
        }
    }
    board.innerHTML = html;
}

function drawBar(id, value) {
    var bar = document.getElementById(id);
    bar.style.width = value + "%";
    if (value < 30) {
        bar.classList.add("low");
    } else {
        bar.classList.remove("low");
    }
}

function drawPanels(state) {
    var levels = ["", "Easy", "Normal", "Hard", "Custom"];
    document.getElementById("playerTitle").innerText =
        playerEmoji[state.player.kind] + " " + state.name + " the " + state.player.animal + " (" + levels[state.level] + ")";
    document.getElementById("abilityText").innerText = state.player.info;
    document.getElementById("livesText").innerText = state.lives;
    document.getElementById("dayText").innerText = state.day;
    document.getElementById("goalText").innerText = state.goal;
    document.getElementById("scoreText").innerText = state.score;

    drawBar("foodBar", state.food);
    drawBar("waterBar", state.water);

    var status = "";
    if (state.hidden) status += "🙈 You are hidden. ";
    if (state.drought > 0) status += "☀️ Drought: " + state.drought + " days left. ";
    if (state.food < 30) status += "Very hungry! ";
    if (state.water < 30) status += "Very thirsty! ";
    document.getElementById("statusText").innerText = status;

    document.getElementById("forecastText").innerText = state.forecast == "" ? "Nothing planned" : state.forecast;

    // radar (already sorted by C insertion sort)
    var radarHtml = "";
    for (var i = 0; i < state.radar.length; i++) {
        var p = state.radar[i];
        var warn = p.dist <= 3 ? " ⚠️" : "";
        radarHtml += "<li>" + p.name + " - " + p.dist + " steps away" + warn + "</li>";
    }
    if (radarHtml == "") radarHtml = "<li>No hunters around</li>";
    document.getElementById("radarList").innerHTML = radarHtml;

    // log (linked list from C, newest first)
    var logHtml = "";
    for (var j = 0; j < state.log.length; j++) {
        logHtml += "<li>" + state.log[j] + "</li>";
    }
    document.getElementById("logList").innerHTML = logHtml;
}

function drawGameOver(state) {
    var box = document.getElementById("overBox");
    var title = document.getElementById("overTitle");

    if (!state.over) {
        box.classList.add("hidden");
        return;
    }
    box.classList.remove("hidden");
    if (state.won) {
        title.innerText = "🎉 You Win!";
        title.className = "win";
    } else {
        title.innerText = "Game Over";
        title.className = "lose";
    }
    document.getElementById("overText").innerText =
        state.reason + " Final score: " + state.score + " (day " + state.day + ")";
}

function drawScores(data) {
    var table = document.getElementById("scoreTable");
    var html = "<tr><th>#</th><th>Name</th><th>Score</th><th>Days</th></tr>";
    for (var i = 0; i < data.scores.length; i++) {
        var s = data.scores[i];
        html += "<tr><td>" + s.rank + "</td><td>" + s.name + "</td><td>" + s.score + "</td><td>" + s.days + "</td></tr>";
    }
    if (data.scores.length == 0) {
        html += '<tr><td colspan="4">No scores yet. Be the first!</td></tr>';
    }
    table.innerHTML = html;
    document.getElementById("scoreInfo").innerText =
        "Games played: " + data.total + "  |  BST height: " + data.treeHeight;
}

function drawAll(state) {
    if (!state.started) return;
    drawBoard(state);
    drawPanels(state);
    drawGameOver(state);
}
