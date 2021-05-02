import SWWatch from "./component/SWWatch.js";
import SWTime from "./component/SWTime.js";

import { default as ESPWSConnector, SW_MODE } from "./ESPWSConnector.js";

let connectorOptions = {};
connectorOptions.wsUrl =
  new URLSearchParams(window.location.search).get("wsUrl") || undefined;

console.log(connectorOptions);

const espConnector = new ESPWSConnector(connectorOptions);

const startStopResetButton = document.querySelector("#startStopReset");
startStopResetButton.setAttribute("targetMode", SW_MODE.COUNTDOWN);
startStopResetButton.addEventListener("click", () => {
  const targetMode = parseInt(startStopResetButton.getAttribute("targetMode"));
  console.log(targetMode == SW_MODE.COUNTDOWN);
  switch (targetMode) {
    case SW_MODE.COUNTDOWN:
      espConnector.swStartCountdown();
      break;
    case SW_MODE.STOP:
      espConnector.swStop();
      break;
    case SW_MODE.RESET:
      espConnector.swReset();
      break;
  }
});

const lapButton = document.querySelector("#lap");
lapButton.addEventListener("click", () => {
  if (swWatch.running) {
    espConnector.swLap();
  }
});

const lapTimesList = document.querySelector("#lapTimes");

/**
 * @type {SWWatch}
 */
const swWatch = document.querySelector("sw-watch");

function onSWModeChange(mode) {
  // Update Stopwatch
  switch (mode) {
    case SW_MODE.RUNNING:
      if (!swWatch.running) {
        swWatch.start();
        startStopResetButton.setAttribute("targetMode", SW_MODE.STOP);
        startStopResetButton.innerText = "Stop";
        lapButton.disabled = false;
      }
      break;
    case SW_MODE.PING:
    case SW_MODE.PONG:
    case SW_MODE.LAP:
      break;
    case SW_MODE.IDLE:
      swWatch.stop();
      startStopResetButton.setAttribute("targetMode", SW_MODE.RUNNING);
      startStopResetButton.innerText = "Start";
      lapButton.disabled = true;
      break;
    case SW_MODE.COUNTDOWN:
    case SW_MODE.FALSESTART:
    case SW_MODE.STOP:
    case SW_MODE.RESET:
      if (swWatch.running) {
        swWatch.stop();
        startStopResetButton.setAttribute("targetMode", SW_MODE.RESET);
        startStopResetButton.innerText = "Reset";
        lapButton.disabled = true;
      }
      break;
  }

  if (mode == SW_MODE.RESET) {
    swWatch.reset();
    lapButton.disabled = true;
  }
}

function onLapTimesUpdate(lapTimes) {
  console.log(lapTimes)
  lapTimesList.innerHTML = "";
  for (const lapTime of lapTimes) {
    const newSWTime = new SWTime();
    newSWTime.time = lapTime;
    const li = document.createElement('li');
    li.appendChild(newSWTime);
    lapTimesList.appendChild(li);
  }
}

async function main() {
  espConnector.onSWModeChange = onSWModeChange;
  espConnector.onLapTimesUpdate = onLapTimesUpdate;

  await espConnector.connect();

  startStopResetButton.disabled = false;
}

main();
