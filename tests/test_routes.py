import requests
import time

BASE_URL = "http://localhost:8080"  

def test_hello():
    r = requests.get(f"{BASE_URL}/hello")
    assert r.status_code == 200
    assert r.json()["message"] == "Hello World"

def test_page():
    r = requests.get(f"{BASE_URL}/page")
    assert r.status_code == 200
    assert "<h1>Welcome!" in r.text

def test_data():
    r = requests.get(f"{BASE_URL}/data")
    assert r.status_code == 200
    assert "<value>42</value>" in r.text

def test_namaskar():
    r = requests.get(f"{BASE_URL}/namaskar")
    assert r.status_code == 200
    assert "Namaskar!" in r.text

def test_json():
    r = requests.get(f"{BASE_URL}/json")
    assert r.status_code == 200
    assert r.json()["message"] == "Hello JSON"

def test_submit():
    payload = {"data": "TestData"}
    r = requests.post(f"{BASE_URL}/submit", json=payload)
    assert r.status_code == 200
    assert "TestData" in r.text

def test_not_found():
    r = requests.get(f"{BASE_URL}/doesnotexist")
    assert r.status_code == 404

def test_submit_invalid():
    r = requests.post(f"{BASE_URL}/submit", json={})
    assert r.status_code == 400 or "error" in r.text.lower()

def test_simulate_error500():
    r = requests.get(f"{BASE_URL}/simulate/error500")
    assert r.status_code == 500
    assert "Simulated Internal Server Error" in r.text

def test_simulate_error404():
    r = requests.get(f"{BASE_URL}/simulate/error404")
    assert r.status_code == 404
    assert "Simulated Not Found" in r.text

def test_simulate_delay():
    start = time.time()
    r = requests.get(f"{BASE_URL}/simulate/delay")
    elapsed = time.time() - start
    assert r.status_code == 200
    assert "delayed" in r.text.lower()
    assert elapsed >= 2
