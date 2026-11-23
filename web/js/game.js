// game.js - buttons and keyboard
// Flow:  key press -> apiMove("U") -> C++ does one turn -> JSON -> drawAll()

var waiting = false;     // true while we wait for the server (so fast key presses do not pile up)
var gameOver = false;

function showGame() {
    document.getElementById("startBox").classList.add("hidden");
    document.getElementById("gameArea").classList.remove("hidden");
}

function showStart() {
    document.getElementById("startBox").classList.remove("hidden");
    document.getElementById("gameArea").classList.add("hidden");
    document.getElementById("overBox").classList.add("hidden");
}

function loadScores() {
    apiScores().then(drawScores);
}

// which animal radio button is checked?
function chosenAnimal() {
    var radios = document.getElementsByName("animal");
    for (var i = 0; i < radios.length; i++) {
        if (radios[i].checked) return radios[i].value;
    }
    return "R";
}

// numbers from the custom level table
function readCustom() {
    return {
        fox:     document.getElementById("c_fox").value,
        eagle:   document.getElementById("c_eagle").value,
        tiger:   document.getElementById("c_tiger").value,
        leopard: document.getElementById("c_leopard").value,
        snake:   document.getElementById("c_snake").value,
        croc:    document.getElementById("c_croc").value,
        deer:    document.getElementById("c_deer").value
    };
}

// show the custom table only when "4 - Custom" is selected
function levelChanged() {
    var level = document.getElementById("levelInput").value;
    if (level == "4") {
        document.getElementById("customBox").classList.remove("hidden");
    } else {
        document.getElementById("customBox").classList.add("hidden");
    }
}

function startGame() {
    var name = document.getElementById("nameInput").value;
    var level = document.getElementById("levelInput").value;
    var animal = chosenAnimal();

    if (name.trim() == "") {
        document.getElementById("startError").innerText = "Please type your name";
        return;
    }

    // the C++ side checks the numbers again (0 to 3) and sends an error if they are wrong
    apiNewGame(name, animal, level, readCustom()).then(function (state) {
        if (!state.ok) {
            document.getElementById("startError").innerText = state.error;
            return;
        }
        document.getElementById("startError").innerText = "";
        gameOver = false;
        showGame();
        drawAll(state);
    }).catch(function () {
        document.getElementById("startError").innerText = "Can not reach the game server. Is rabbit_run running?";
    });
}

function move(dir) {
    if (waiting || gameOver) return;
    waiting = true;

    apiMove(dir).then(function (state) {
        waiting = false;
        if (!state.ok) {
            console.log(state.error);
            return;
        }
        drawAll(state);
        if (state.over) {
            gameOver = true;
            loadScores();          // C++ already saved the score, so reload the table
        }
    }).catch(function () {
        waiting = false;
        alert("Lost connection to the game server.");
    });
}

// keyboard
document.addEventListener("keydown", function (e) {
    if (document.getElementById("gameArea").classList.contains("hidden")) return;

    var key = e.key;
    var dir = "";
    if (key == "ArrowUp" || key == "w" || key == "W") dir = "U";
    if (key == "ArrowDown" || key == "s" || key == "S") dir = "D";
    if (key == "ArrowLeft" || key == "a" || key == "A") dir = "L";
    if (key == "ArrowRight" || key == "d" || key == "D") dir = "R";
    if (key == " ") dir = "X";

    if (dir != "") {
        e.preventDefault();      // stop the page from scrolling
        move(dir);
    }
});

// on-screen buttons
var padButtons = document.querySelectorAll("#pad button");
for (var i = 0; i < padButtons.length; i++) {
    padButtons[i].addEventListener("click", function () {
        move(this.getAttribute("data-dir"));
    });
}

document.getElementById("startBtn").addEventListener("click", startGame);
document.getElementById("levelInput").addEventListener("change", levelChanged);
document.getElementById("againBtn").addEventListener("click", showStart);

// when the page opens: show the scores, and if a game is already running, continue it
loadScores();
apiState().then(function (state) {
    if (state.started && !state.over) {
        showGame();
        drawAll(state);
    }
}).catch(function () {
    document.getElementById("startError").innerText = "Can not reach the game server. Is rabbit_run running?";
});
