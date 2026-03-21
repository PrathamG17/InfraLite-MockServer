import requests
import time
import re
import os

BASE_URL = "https://localhost:8080"
LOG_FILE = os.path.join("logs", "InfraLite-MockServer.log")

def get_logged_token():
    """Read the last logged JWT token from the server log."""
    if not os.path.exists(LOG_FILE):
        raise FileNotFoundError("Log file not found, cannot extract token")
    with open(LOG_FILE, "r", encoding="utf-8") as f:
        content = f.read()
    match = re.findall(r"Generated test JWT token: (.+)", content)
    if not match:
        raise ValueError("No token found in log file")
    return match[-1].strip()

# --- Existing tests ---

def test_hello():
    r = requests.get(f"{BASE_URL}/hello", verify=False)
    assert r.status_code == 200
    assert r.json()["message"] == "Hello World"

def test_page():
    r = requests.get(f"{BASE_URL}/page", verify=False)
    assert r.status_code == 200
    assert "<h1>Welcome!" in r.text

def test_data():
    r = requests.get(f"{BASE_URL}/data", verify=False)
    assert r.status_code == 200
    assert "<value>42</value>" in r.text

def test_namaskar():
    r = requests.get(f"{BASE_URL}/namaskar", verify=False)
    assert r.status_code == 200
    assert "Namaskar!" in r.text

def test_json():
    r = requests.get(f"{BASE_URL}/json", verify=False)
    assert r.status_code == 200
    assert r.json()["message"] == "Hello JSON"

def test_not_found():
    r = requests.get(f"{BASE_URL}/doesnotexist", verify=False)
    assert r.status_code == 404

def test_submit():
    r = requests.post(f"{BASE_URL}/submit", json={"data": "TestData"}, verify=False)
    assert r.status_code == 200
    assert "TestData" in r.text

def test_submit_invalid():
    r = requests.post(f"{BASE_URL}/submit", json={}, verify=False)
    assert r.status_code == 400
    assert "error" in r.text.lower()

def test_simulate_error500():
    r = requests.get(f"{BASE_URL}/simulate/error500", verify=False)
    assert r.status_code == 500
    assert "Simulated Internal Server Error" in r.text

def test_simulate_error404():
    r = requests.get(f"{BASE_URL}/simulate/error404", verify=False)
    assert r.status_code == 404
    assert "Simulated Not Found" in r.text

def test_simulate_delay():
    start = time.time()
    r = requests.get(f"{BASE_URL}/simulate/delay", verify=False)
    elapsed = time.time() - start
    assert r.status_code == 200
    assert "delayed" in r.text.lower()
    assert elapsed >= 2

# --- New JWT/TLS tests ---

def test_secure_admin_with_token():
    token = get_logged_token()
    headers = {"Authorization": f"Bearer {token}"}
    r = requests.get(f"{BASE_URL}/secure/admin", headers=headers, verify=False)
    assert r.status_code == 200
    assert "admin" in r.text.lower()

def test_secure_admin_without_token():
    r = requests.get(f"{BASE_URL}/secure/admin", verify=False)
    assert r.status_code == 403 or "denied" in r.text.lower()

def test_secure_qa_with_token():
    token = get_logged_token()
    headers = {"Authorization": f"Bearer {token}"}
    r = requests.get(f"{BASE_URL}/secure/qa", headers=headers, verify=False)
    assert r.status_code == 403

def test_secure_viewer_with_token():
    token = get_logged_token()
    headers = {"Authorization": f"Bearer {token}"}
    r = requests.get(f"{BASE_URL}/secure/viewer", headers=headers, verify=False)
    assert r.status_code == 403
    