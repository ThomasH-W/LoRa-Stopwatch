export const SYS_MODE = {
  STOPWATCH: 0,
  STARTLOOP: 1,
  ADMIN: 2,
  BOOT: 3,
};

export const SW_MODE = {
  IDLE: 0,
  COUNTDOWN: 1,
  RUNNING: 2,
  FALSESTART: 3,
  LAP: 4,
  STOP: 5,
  RESET: 6,
  PING: 7,
  PONG: 8,
};

export default class ESPWSConnector {
  /**
   * @type {WebSocket}
   */
  _ws;
  _wsUrl;

  /**
   * @type {Number => Any}
   */
  onSWTimerUpdate;

  /**
   * @type {Number => Any}
   */
  onLapTimesUpdate;

  /**
   * @type {Number => Any}
   */
  onSWModeChange;

  /**
   * @type {Number => Any}
   */
  onSysModeChange;

  _sys_mode;
  _sw_mode;
  _sw_laps_used;
  _lap_times;
  _admin = {};

  constructor({ wsUrl = `ws://${window.location.host}` } = {}) {
    this._wsUrl = wsUrl;
  }

  connect() {
    return new Promise((res) => {
      this._ws = new WebSocket(this._wsUrl);
      this._ws.addEventListener('message', (msg) => this.onMessage(msg.data));
      this._ws.addEventListener('open', res);
      this._ws.addEventListener('error', (e) => this.onError(e));
    });
  }

  get sw_mode() {
    return this._sw_mode;
  }

  get sys_mode() {
    return this._sys_mode;
  }

  onError(e) {
    console.error(e);
  }

  onMessage(msg) {
    console.log('Recieved:', msg);
    const commands = msg.split(';').map((s) => s.trim());
    for (const command of commands) {
      const [cmdName, ...argParts] = command.split('=');
      const cmdArgs = argParts.join('=');
      this.handleCommand(cmdName, cmdArgs);
    }
  }

  handleCommand(cmd, args) {
    console.log('Hadnling Command', {
      cmd,
      args,
    });
    switch (cmd) {
      case 'sw_mode':
        switch (parseInt(args)) {
          case SYS_MODE.LAP:
          case SYS_MODE.PING:
          case SYS_MODE.PONG:
            break;
          default:
            this._sw_mode = parseInt(args);
            if (this.onSWModeChange) {
              this.onSWModeChange(this._sw_mode);
            }
        }
        break;
      case 'sys_mode':
        this._sys_mode = parseInt(args);
        if (this.onSysModeChange) {
          this.onSysModeChange(this._sys_mode);
        }
        break;
      case 'sw_timer':
        if (this.onSWTimerUpdate) {
          this.onSWTimerUpdate(parseInt(args));
        }
        break;
      case 'sw_laps_used':
        this._sw_laps_used = parseInt(args);
        break;
      case 'timer':
        console.log(args.split(','));
        this._lap_times = args
          .split(',')
          .map((t) => parseInt(t))
          .filter((t) => t != 0)
          .reverse();
        this.onLapTimesUpdate(this._lap_times);
        break;
      case 'admin_firmware':
        this._admin.firmware = args;
        break;
      case 'admin_deviceID':
        this._admin.deviceID = args;
        break;
      case 'admin_RSSI':
        this._admin.RSSI = args;
        break;
      case 'admin_SNR':
        this._admin.SNR = args;
        break;
      case 'admin_roundtrip':
        this._admin.roundtrip = args;
        break;
      default:
        console.log('Unknown Command!');
    }
  }

  send(msg) {
    console.log('Sending', msg);
    this._ws.send(msg);
  }

  swStartCountdown() {
    this.send(`sw_mode=${SW_MODE.COUNTDOWN}`);
  }

  swLap() {
    this.send(`sw_mode=${SW_MODE.LAP}`);
  }

  swStop() {
    this.send(`sw_mode=${SW_MODE.STOP}`);
  }

  swReset() {
    this.send(`sw_mode=${SW_MODE.RESET}`);
  }

  sysSetMode(mode) {
    this.send(`sys_mode=${mode}`);
  }
}
