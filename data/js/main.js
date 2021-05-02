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

const sysModeSelect = document.querySelector("#sys_mode_select");
sysModeSelect.addEventListener("change", () =>
  espConnector.sysSetMode(sysModeSelect.value)
);

const lapTimesList = document.querySelector("#lapTimes tbody");

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
        swWatch.countdown = 0;
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
      startStopResetButton.setAttribute("targetMode", SW_MODE.COUNTDOWN);
      startStopResetButton.innerText = "Start";
      lapButton.disabled = true;
      onLapTimesUpdate([]);
      break;
    case SW_MODE.COUNTDOWN:
    case SW_MODE.FALSESTART:
    case SW_MODE.STOP:
    case SW_MODE.RESET:
      if (swWatch.running) {
        swWatch.stop();
      }
      startStopResetButton.setAttribute("targetMode", SW_MODE.RESET);
      startStopResetButton.innerText = "Reset";
      lapButton.disabled = true;
      break;
  }

  if (mode == SW_MODE.IDLE) {
    swWatch.reset();
    lapButton.disabled = true;
  }
}

function onLapTimesUpdate(lapTimes) {
  console.log(lapTimes);
  lapTimesList.innerHTML = "";
  for (let i = 0; i < lapTimes.length; i++) {
    lapTimesList.innerHTML += `
      <tr>
        <td>${i + 1}</td>
        <td>${
          i > 0
            ? "<sw-time time=" + (lapTimes[i] - lapTimes[i - 1]) + "></sw-time>"
            : ""
        }</td>
        <td><sw-time time="${lapTimes[i]}"></sw-time></td>
      </tr>
    `;
  }
}

function onSWCountdownUpdate(countdownValue) {
  swWatch.countdown = countdownValue;
}

function onSWTimerUpdate(time) {
  swWatch.time = time;
}

function onAdminInfoUpdate(adminInfo) {
  document.getElementById("sys_info__firmware").innerText =
    adminInfo.firmware || "-";
  document.getElementById("sys_info__id").innerText = adminInfo.deviceID || "-";
  document.getElementById("sys_info__rssi").innerText = adminInfo.RSSI || "-";
  document.getElementById("sys_info__snr").innerText = adminInfo.SNR || "-";
  document.getElementById("sys_info__ping").innerText =
    adminInfo.roundtrip || "-";
}

function onSysModeChange(sys_mode) {
  document.getElementById("sys_mode_select").value = sys_mode;
}

async function main() {
  espConnector.onSWTimerUpdate = onSWTimerUpdate;
  espConnector.onSWModeChange = onSWModeChange;
  espConnector.onSysModeChange = onSysModeChange;
  espConnector.onLapTimesUpdate = onLapTimesUpdate;
  espConnector.onSWCountdownUpdate = onSWCountdownUpdate;
  espConnector.onAdminInfoUpdate = onAdminInfoUpdate;

  await espConnector.connect();

  startStopResetButton.disabled = false;
}

main();
