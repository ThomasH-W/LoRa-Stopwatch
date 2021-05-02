const swTimeTemplate = document.createElement("template");
swTimeTemplate.innerHTML = `
  <span id="minutes">00</span> :
  <span id="seconds">00</span> :
  <span id="hundreds">00</span>
`;

export default class SWTime extends HTMLElement {
  static get observedAttributes() {
    return ["time"];
  }

  constructor() {
    // Always call super first in constructor
    super();
    let shadowRoot = this.attachShadow({ mode: "open" });
    shadowRoot.appendChild(swTimeTemplate.content.cloneNode(true));
    // write element functionality in here
  }

  set time(value) {
    this.setAttribute("time", value);
  }

  get time() {
    return parseInt(this.getAttribute("time"));
  }

  update() {
    let time = this.time;
    const hundreds = Math.floor(time/10) % 100 + ""
    const seconds = Math.floor(time/1000) % 60 + ""
    const minutes = Math.floor(time/60000) + ""
    this.shadowRoot.getElementById("hundreds").innerText = hundreds.padStart(2, '0');
    this.shadowRoot.getElementById("seconds").innerText = seconds.padStart(2, '0');
    this.shadowRoot.getElementById("minutes").innerText = minutes.padStart(2, '0');
  }

  attributeChangedCallback(name, newValue) {
    // When the drawer is disabled, update keyboard/screen reader behavior.
    switch (name) {
      case "time":
        this.update();
        break;
    }
  }
}

window.customElements.define("sw-time", SWTime);
