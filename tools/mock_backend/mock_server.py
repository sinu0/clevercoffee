import json
import math
import random
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional

from flask import Flask, Response, jsonify, redirect, request, send_from_directory


BASE_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BASE_DIR.parent.parent
FRONTEND_DIR = PROJECT_DIR / "frontend"
HTML_DIR = FRONTEND_DIR / "html"
HTML_FRAGMENTS_DIR = FRONTEND_DIR / "html_fragments"
STATIC_DIRS = {
    "css": FRONTEND_DIR / "css",
    "js": FRONTEND_DIR / "js",
    "img": FRONTEND_DIR / "img",
    "webfonts": FRONTEND_DIR / "webfonts",
}

app = Flask(__name__)


@dataclass
class Parameter:
    name: str
    displayName: str
    type: int
    section: int
    position: int
    value: Any
    min: float = 0.0
    max: float = 0.0
    hasHelpText: bool = True
    show: bool = True
    reboot: bool = False
    options: Optional[List[Dict[str, Any]]] = None
    helpText: str = ""
    show_if: Optional[str] = None


PARAMS: List[Parameter] = [
    Parameter(
        name="pid.enabled",
        displayName="PID Controller",
        type=1,
        section=0,
        position=11,
        value=1,
        min=0,
        max=1,
        helpText="Enable or disable the PID controller.",
    ),
    Parameter(
        name="brew.setpoint",
        displayName="Brew Setpoint (°C)",
        type=2,
        section=1,
        position=201,
        value=95.0,
        min=20.0,
        max=110.0,
        helpText="Target brew temperature.",
    ),
    Parameter(
        name="brew.mode",
        displayName="Brew Mode",
        type=5,
        section=3,
        position=301,
        value=0,
        options=[{"value": 0, "label": "Manual"}, {"value": 1, "label": "Automatic"}],
        helpText="Manual or Automatic brew mode.",
    ),
    Parameter(
        name="brew.by_time.enabled",
        displayName="Brew by Time",
        type=1,
        section=3,
        position=311,
        value=0,
        min=0,
        max=1,
        helpText="Enable brew-by-time when in Automatic mode.",
    ),
    Parameter(
        name="brew.by_time.target_time",
        displayName="Target Brew Time (s)",
        type=2,
        section=3,
        position=312,
        value=25.0,
        min=1.0,
        max=120.0,
        helpText="Stop brew after this time.",
    ),
    Parameter(
        name="brew.pre_infusion.enabled",
        displayName="Pre-Infusion",
        type=1,
        section=3,
        position=331,
        value=0,
        min=0,
        max=1,
        helpText="Enable pre-infusion.",
    ),
    Parameter(
        name="brew.pre_infusion.mode",
        displayName="Pre-infusion Mode",
        type=5,
        section=3,
        position=330,
        value=0,
        options=[{"value": 0, "label": "Single"}, {"value": 1, "label": "Pulse"}],
        helpText="Selects pre-infusion mode.",
    ),
    Parameter(
        name="brew.pre_infusion.time",
        displayName="Pre-infusion Time (s)",
        type=2,
        section=3,
        position=332,
        value=2.0,
        min=0.0,
        max=60.0,
        helpText="Time the pump runs during single pre-infusion.",
    ),
    Parameter(
        name="brew.pre_infusion.pause",
        displayName="Pre-infusion Pause Time (s)",
        type=2,
        section=3,
        position=333,
        value=5.0,
        min=0.0,
        max=60.0,
        helpText="Pause after single pre-infusion.",
    ),
    Parameter(
        name="brew.pre_infusion.pulse.on_time",
        displayName="Pulse ON Time (s)",
        type=2,
        section=3,
        position=334,
        value=0.6,
        min=0.0,
        max=10.0,
        helpText="Pump ON time for each pulse.",
        show_if="pulse",
    ),
    Parameter(
        name="brew.pre_infusion.pulse.off_time",
        displayName="Pulse OFF Time (s)",
        type=2,
        section=3,
        position=335,
        value=0.8,
        min=0.0,
        max=10.0,
        helpText="Pump OFF time between pulses.",
        show_if="pulse",
    ),
    Parameter(
        name="brew.pre_infusion.pulse.cycles",
        displayName="Pulse Cycles",
        type=0,
        section=3,
        position=336,
        value=3,
        min=1,
        max=10,
        helpText="Number of pulses.",
        show_if="pulse",
    ),
    Parameter(
        name="brew.pre_infusion.soak_time",
        displayName="Soak Time (s)",
        type=2,
        section=3,
        position=337,
        value=5.0,
        min=0.0,
        max=60.0,
        helpText="Soak time after pulses.",
        show_if="pulse",
    ),
    Parameter(
        name="STEAM_MODE",
        displayName="Steam Mode",
        type=1,
        section=10,
        position=501,
        value=0,
        min=0,
        max=1,
        helpText="Toggle steam mode.",
        hasHelpText=False,
    ),
    Parameter(
        name="BACKFLUSH_ON",
        displayName="Backflush Mode",
        type=1,
        section=10,
        position=502,
        value=0,
        min=0,
        max=1,
        helpText="Toggle backflush mode.",
        hasHelpText=False,
    ),
    Parameter(
        name="TARE_ON",
        displayName="Tare Scale",
        type=1,
        section=10,
        position=503,
        value=0,
        min=0,
        max=1,
        helpText="Tare the scale.",
        hasHelpText=False,
    ),
    Parameter(
        name="CALIBRATION_ON",
        displayName="Scale Calibration",
        type=1,
        section=10,
        position=504,
        value=0,
        min=0,
        max=1,
        helpText="Start scale calibration.",
        hasHelpText=False,
    ),
]

PARAM_BY_NAME: Dict[str, Parameter] = {p.name: p for p in PARAMS}


def get_param_value(name: str) -> Any:
    param = PARAM_BY_NAME.get(name)
    return param.value if param else None


def set_param_value(name: str, value: Any) -> None:
    param = PARAM_BY_NAME.get(name)
    if not param:
        return
    if param.type in (0, 1):
        param.value = int(value)
    elif param.type in (2, 3):
        param.value = float(value)
    else:
        param.value = value


def is_param_visible(param: Parameter) -> bool:
    if not param.show:
        return False
    if param.show_if == "pulse":
        return int(get_param_value("brew.pre_infusion.mode") or 0) == 1
    return True


def param_to_json(param: Parameter) -> Dict[str, Any]:
    doc = {
        "type": param.type,
        "name": param.name,
        "displayName": param.displayName,
        "section": param.section,
        "position": param.position,
        "hasHelpText": param.hasHelpText,
        "show": is_param_visible(param),
        "reboot": param.reboot,
        "value": param.value,
        "min": param.min,
        "max": param.max,
    }
    if param.type == 5 and param.options:
        doc["options"] = param.options
    return doc


def filter_parameters(filter_type: str) -> List[Parameter]:
    result: List[Parameter] = []
    for param in PARAMS:
        if not is_param_visible(param):
            continue
        include = False
        if filter_type == "hardware":
            include = 11 <= param.section <= 15
        elif filter_type == "behavior":
            include = 0 <= param.section <= 9
        elif filter_type == "other":
            include = param.section == 10
        elif filter_type == "all":
            include = True
        else:
            include = param.section in (0, 1, 10)
        if include:
            result.append(param)
    return result


@app.route("/parameters", methods=["GET", "POST"])
def parameters():
    if request.method == "GET":
        filter_type = request.args.get("filter", "")
        offset = int(request.args.get("offset", "0"))
        limit = int(request.args.get("limit", "5"))

        filtered = filter_parameters(filter_type)
        sliced = filtered[offset:offset + limit]
        payload = {
            "parameters": [param_to_json(p) for p in sliced],
            "offset": offset,
            "limit": limit,
            "returned": len(sliced),
        }
        return jsonify(payload)

    for key, value in request.form.items():
        if not value:
            continue
        if key in PARAM_BY_NAME:
            set_param_value(key, value)

    return Response("OK", mimetype="text/plain")


@app.route("/parameterHelp", methods=["GET"])
def parameter_help():
    name = request.args.get("param")
    if not name:
        return Response("parameter is missing", status=422)
    param = PARAM_BY_NAME.get(name)
    if not param:
        return Response("parameter not found", status=404)
    return jsonify({"name": name, "helpText": param.helpText})


@app.route("/toggleSteam", methods=["POST"])
def toggle_steam():
    set_param_value("STEAM_MODE", 1 - int(get_param_value("STEAM_MODE") or 0))
    return redirect("/")


@app.route("/togglePid", methods=["POST"])
def toggle_pid():
    set_param_value("pid.enabled", 1 - int(get_param_value("pid.enabled") or 0))
    return redirect("/")


@app.route("/toggleBackflush", methods=["POST"])
def toggle_backflush():
    set_param_value("BACKFLUSH_ON", 1 - int(get_param_value("BACKFLUSH_ON") or 0))
    return redirect("/")


@app.route("/toggleTareScale", methods=["POST"])
def toggle_tare():
    set_param_value("TARE_ON", 1 - int(get_param_value("TARE_ON") or 0))
    return redirect("/")


@app.route("/toggleScaleCalibration", methods=["POST"])
def toggle_calibration():
    set_param_value("CALIBRATION_ON", 1 - int(get_param_value("CALIBRATION_ON") or 0))
    return redirect("/")


@app.route("/download/config", methods=["GET"])
def download_config():
    config = {p.name: p.value for p in PARAMS}
    return Response(json.dumps(config, indent=2), mimetype="application/json")


@app.route("/upload/config", methods=["POST"])
def upload_config():
    file = request.files.get("config")
    if not file:
        return jsonify({"success": False, "message": "No config file uploaded.", "restart": True}), 400

    try:
        uploaded = json.loads(file.read().decode("utf-8"))
    except json.JSONDecodeError:
        return jsonify({"success": False, "message": "Invalid JSON.", "restart": True}), 400

    for key, value in uploaded.items():
        if key in PARAM_BY_NAME:
            set_param_value(key, value)

    return jsonify({"success": True, "message": "Configuration validated and applied successfully.", "restart": True})


@app.route("/restart", methods=["POST"])
def restart():
    return Response("Restarting...", mimetype="text/plain")


@app.route("/factoryreset", methods=["POST"])
def factory_reset():
    for param in PARAMS:
        if isinstance(param.value, (int, float)):
            param.value = param.min if param.min is not None else param.value
    return Response("Factory reset. Restarting...", mimetype="text/plain")


@app.route("/wifireset", methods=["POST"])
def wifi_reset():
    return Response("WiFi settings are being reset. Rebooting...", mimetype="text/plain")


@app.route("/temperatures", methods=["GET"])
def temperatures():
    return jsonify(TEMP_STATE.copy())


@app.route("/timeseries", methods=["GET"])
def timeseries():
    return jsonify(
        {
            "currentTemps": HISTORY["currentTemps"],
            "targetTemps": HISTORY["targetTemps"],
            "heaterPowers": HISTORY["heaterPowers"],
        }
    )


@app.route("/events")
def events():
    def stream():
        while True:
            payload = json.dumps(TEMP_STATE)
            yield f"event: new_temps\ndata: {payload}\n\n"
            time.sleep(1)

    return Response(stream(), mimetype="text/event-stream")


@app.route("/manifest.json")
def manifest():
    return send_from_directory(FRONTEND_DIR, "manifest.json")


@app.route("/js/<path:path>")
def serve_js(path: str):
    return send_from_directory(STATIC_DIRS["js"], path)


@app.route("/css/<path:path>")
def serve_css(path: str):
    return send_from_directory(STATIC_DIRS["css"], path)


@app.route("/img/<path:path>")
def serve_img(path: str):
    return send_from_directory(STATIC_DIRS["img"], path)


@app.route("/webfonts/<path:path>")
def serve_webfonts(path: str):
    return send_from_directory(STATIC_DIRS["webfonts"], path)


def render_html(filename: str):
    header = (HTML_FRAGMENTS_DIR / "header.html").read_text(encoding="utf-8")
    html = (HTML_DIR / filename).read_text(encoding="utf-8")
    return html.replace("%HEADER%", header)


@app.route("/")
@app.route("/index.html")
def index():
    return Response(render_html("index.html"), mimetype="text/html")


@app.route("/parameters.html")
def parameters_page():
    return Response(render_html("parameters.html"), mimetype="text/html")


@app.route("/system.html")
def system_page():
    return Response(render_html("system.html"), mimetype="text/html")


@app.route("/about.html")
def about_page():
    return Response(render_html("about.html"), mimetype="text/html")


TEMP_STATE = {"currentTemp": 93.0, "targetTemp": 95.0, "heaterPower": 45.0}
HISTORY = {"currentTemps": [], "targetTemps": [], "heaterPowers": []}
HISTORY_MAX = 200


def update_temps_loop():
    t = 0.0
    while True:
        t += 0.1
        target = float(get_param_value("brew.setpoint") or 95.0)
        current = target + math.sin(t) * 1.2 + random.uniform(-0.2, 0.2)
        heater = max(0.0, min(100.0, 50.0 + math.sin(t * 0.7) * 20.0))

        TEMP_STATE["currentTemp"] = round(current, 2)
        TEMP_STATE["targetTemp"] = round(target, 2)
        TEMP_STATE["heaterPower"] = round(heater, 2)

        HISTORY["currentTemps"].append(TEMP_STATE["currentTemp"])
        HISTORY["targetTemps"].append(TEMP_STATE["targetTemp"])
        HISTORY["heaterPowers"].append(TEMP_STATE["heaterPower"])

        for key in HISTORY:
            if len(HISTORY[key]) > HISTORY_MAX:
                HISTORY[key] = HISTORY[key][-HISTORY_MAX:]

        time.sleep(1)


if __name__ == "__main__":
    thread = threading.Thread(target=update_temps_loop, daemon=True)
    thread.start()
    app.run(host="0.0.0.0", port=8000, debug=False)
