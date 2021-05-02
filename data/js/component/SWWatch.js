import "./SWTime.js";

const swWatchTemplate = document.createElement("template");
swWatchTemplate.innerHTML = `
  <style>
    :host {
      font-family: Calibri;
      width: 20rem;
      height: 20rem;
      display: grid;
      align-items: center;
      justify-content: center;
      color: #fff;
      background: radial-gradient(#111 68%, transparent 0), conic-gradient(#a00, #a00 calc(var(--minute-progress, 0) * 360deg), transparent 0deg), #111;
      border-radius: 50%;
      font-variant-numeric: tabular-nums;
      font-size: 3rem;
      position: relative;
      overflow: hidden;
    }

    sw-time {
      z-index: 0;
    }

    #redRing {
      position: absolute;
      width: 100%;
      height: 100%;
      background: #333;
      clip-path: circle(48% at 50% 50%);
    }

    #tenMinuteMarkers, #minuteMarkers {
      position: absolute;
      width: 100%;
      height: 100%;
    }

    #tenMinuteMarkers div,
    #minuteMarkers div {
      width: .5%;
      height: 100%;
      background: #fff;
      position: absolute;
      left: 50%;
      transform-origin: 50% 50%;
      transform: translateX(-50%) rotate(var(--rotation)) translateY(100%) translateY(-2%);
    }

    #tenMinuteMarkers div {
      width: 1%;
      transform: translateX(-50%) rotate(var(--rotation)) translateY(100%) translateY(-5%);
    }
  </style>
  <!--<div id="redRing"></div>-->
  <div id="tenMinuteMarkers"></div>
  <div id="minuteMarkers"></div>
  <sw-time time=0></sw-time>
`;

const minuteMarkers = swWatchTemplate.content.querySelector("#minuteMarkers");
for (let i = 0; i < 60; i++) {
  const newDiv = document.createElement("div");
  newDiv.style.setProperty("--rotation", `${i * 6}deg`);
  minuteMarkers.appendChild(newDiv);
}

const tenMinuteMarkers = swWatchTemplate.content.querySelector(
  "#tenMinuteMarkers"
);
for (let i = 0; i < 12; i++) {
  const newDiv = document.createElement("div");
  newDiv.style.setProperty("--rotation", `${i * 30}deg`);
  tenMinuteMarkers.appendChild(newDiv);
}

export default class SWWatch extends HTMLElement {
  static get observedAttributes() {
    return ["running"];
  }

  _start = 0;
  _time = 0;

  constructor() {
    // Always call super first in constructor
    super();
    let shadowRoot = this.attachShadow({ mode: "open" });
    shadowRoot.appendChild(swWatchTemplate.content.cloneNode(true));
    // write element functionality in here
  }

  set running(value) {
    if (value) {
      this.setAttribute("running", "");
    } else {
      this.removeAttribute("running");
    }
  }

  get running() {
    return this.hasAttribute("running");
  }

  get time() {
    return this._time;
  }

  start() {
    this._start = performance.now();
    this.running = true;
  }

  stop() {
    this.running = false;
  }

  reset() {
    this.running = false;
    this._time = 0;
    this.updateUI(); // Update once to reflect 0 in UI
  }

  updateUI() {
    this.shadowRoot.querySelector("sw-time").time = this._time;
    this.style.setProperty("--minute-progress", (this._time / 60000) % 1);
  }

  rafLoop() {
    if (this.running) {
      this._time = performance.now() - this._start;
      this.updateUI();
      requestAnimationFrame((_) => this.rafLoop());
    }
  }

  update() {
    if (this.running) {
      requestAnimationFrame((_) => this.rafLoop());
    }
  }

  attributeChangedCallback(name, newValue) {
    // When the drawer is disabled, update keyboard/screen reader behavior.
    switch (name) {
      case "running":
        this.update();
        break;
    }
  }
}

window.customElements.define("sw-watch", SWWatch);
