// api.js - talks to the C++ program
// Every function sends one GET request and gives back the JSON answer.

function callServer(url) {
    return fetch(url).then(function (response) {
        return response.json();
    });
}

function apiState() {
    return callServer("/api/state");
}

// custom = object with fox, eagle, tiger, leopard, snake, croc, deer (only used for level 4)
function apiNewGame(name, animal, level, custom) {
    var url = "/api/new?name=" + encodeURIComponent(name) + "&animal=" + animal + "&level=" + level;
    if (level == 4) {
        url += "&fox=" + custom.fox + "&eagle=" + custom.eagle + "&tiger=" + custom.tiger +
               "&leopard=" + custom.leopard + "&snake=" + custom.snake +
               "&croc=" + custom.croc + "&deer=" + custom.deer;
    }
    return callServer(url);
}

function apiMove(dir) {
    return callServer("/api/move?dir=" + dir);
}

function apiScores() {
    return callServer("/api/scores");
}
