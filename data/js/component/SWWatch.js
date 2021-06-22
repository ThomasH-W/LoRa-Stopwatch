import "./SWTime.js";

const swWatchTemplate = document.createElement("template");
swWatchTemplate.innerHTML = `
  <style>
    :host {
      width: 20rem;
      height: 20rem;
      display: grid;
      grid-template-rows: 1fr auto auto 1fr;
      grid-template-areas: "." "total" "current" ".";
      align-items: center;
      justify-content: center;
      color: #fff;
      background: radial-gradient(#111 50%, transparent 70%), conic-gradient(var(--intense-theme-color), var(--intense-theme-color) calc(var(--minute-progress, 0) * 360deg), transparent 0deg), #111;
      border-radius: 50%;
      font-variant-numeric: tabular-nums;
      position: relative;
      overflow: hidden;
      gap: 1rem;
    }
    
    sw-time {
      font-size: 3em;
      z-index: 0;
      transition: filter .1s;
      text-align: center;
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

    :host([countdown]) sw-time {
      filter: blur(1rem);
    }

    #countdown {
      position: absolute;
      left: 50%;
      font-size: 12em;
      transform: translate(-50%);
      opacity: 0;
      filter: blur(1rem);
      transition: opacity .1s filter .1s;
    }

    :host([countdown]) #countdown {
      opacity: 1;
      filter: blur(0);
    }

    #total {
      grid-area: total;
    }

    #currentLap {
      grid-area: current;
      font-size: 2em;
      color: #aaa;
    }
  </style>
  <!--<div id="redRing"></div>-->
  <div id="tenMinuteMarkers"></div>
  <div id="minuteMarkers"></div>
  <div id="countdown">10</div>
  <sw-time id="total" time=0></sw-time>
  <sw-time id="currentLap" time=0></sw-time>
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
    return ["running", "countdown"];
  }

  _start = 0;
  _time = 0;
  lastLap = 0;

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

  get countdown() {
    return parseInt(this.getAttribute("countdown") || 0);
  }

  set countdown(value) {
    if(value > 0){
      this.setAttribute("countdown", value);
    } else {
      this.removeAttribute("countdown");
    }
  }

  get time() {
    return this._time;
  }

  set time(value) {
    this._start += value - this.time;
    this._time = value;
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
    this.lastLap = 0;
    this.updateUI(); // Update once to reflect 0 in UI
  }

  updateUI() {
    this.shadowRoot.querySelector("sw-time#total").time = this._time;
    this.shadowRoot.querySelector("sw-time#currentLap").time = this._time - this.lastLap;
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
    const countdown = this.countdown;
    if(countdown) {
      this.shadowRoot.querySelector("#countdown").innerText = countdown;
    }
    if (this.running) {
      requestAnimationFrame((_) => this.rafLoop());
    }
  }

  attributeChangedCallback(name, newValue) {
    // When the drawer is disabled, update keyboard/screen reader behavior.
    switch (name) {
      case "running":
      case "countdown":
        this.update();
        break;
    }
  }
}

window.customElements.define("sw-watch", SWWatch);
