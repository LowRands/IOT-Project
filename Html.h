#ifndef HTML_H
#define HTML__H

//PROGMEM tells the compiler to store data in program memory (flash) instead of RAM
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Glass Bottle Color Sorter</title>
  <style>
    body { font-family: Arial, sans-serif; background: #1a1a2e; color: #eee; margin: 0; padding: 20px; }
    .container { max-width: 600px; margin: 0 auto; }
    h1 { text-align: center; color: #4fbdba; }
    .card { background: #16213e; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
    .current { font-size: 28px; font-weight: bold; padding: 20px; border-radius: 8px; text-align: center; margin-bottom: 15px; }
    .clear { background: #e6eef6; color: #1a1a2e; }
    .green { background: #22c55e; color: white; }
    .brown { background: #644117; color: white; }
    .unknown { background: #666; color: white; }
    .none { background: #333; color: #aaa; }
    .counter-row { display: flex; align-items: center; padding: 12px; margin: 8px 0; background: #0f3460; border-radius: 5px; }
    .color-box { width: 30px; height: 30px; border-radius: 5px; margin-right: 15px; }
    .counter-name { flex: 1; font-weight: bold; }
    .counter-value { font-size: 24px; font-weight: bold; color: #4fbdba; }
    .total { text-align: center; font-size: 20px; margin-top: 15px; color: #4fbdba; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Advanced Bottle Bank</h1>
    <div class="card">
      <div id="currentDisplay" class="current none">Waiting...</div>
      <p style="text-align:center; color:#aaa;">Last detected bottle</p>
    </div>
    <div class="card">
      <h3 style="margin-top:0;">Color Counts</h3>
      <div class="counter-row">
        <div class="color-box" style="background:#e6eef6;"></div>
        <div class="counter-name">Clear</div>
        <div class="counter-value" id="countClear">0</div>
      </div>
      <div class="counter-row">
        <div class="color-box" style="background:#22c55e;"></div>
        <div class="counter-name">Green</div>
        <div class="counter-value" id="countGreen">0</div>
      </div>
      <div class="counter-row">
        <div class="color-box" style="background:#644117;"></div>
        <div class="counter-name">Brown</div>
        <div class="counter-value" id="countBrown">0</div>
      </div>
      <div class="counter-row">
        <div class="color-box" style="background:#666;"></div>
        <div class="counter-name">Unknown</div>
        <div class="counter-value" id="countOther">0</div>
      </div>
      <div class="total">Total: <span id="totalCount">0</span></div>
    </div>
  </div>
  <script>
    function updateData() {
      fetch('/data')
        .then(r => r.json())
        .then(d => {
          document.getElementById('countClear').textContent = d.clear;
          document.getElementById('countGreen').textContent = d.green;
          document.getElementById('countBrown').textContent = d.brown;
          document.getElementById('countOther').textContent = d.other;
          document.getElementById('totalCount').textContent = d.total;
          
          const display = document.getElementById('currentDisplay');
          display.textContent = d.last;
          display.className = 'current ' + d.last.toLowerCase();
        })
        .catch(e => console.error('Update failed:', e));
    }
    setInterval(updateData, 1000);
    updateData();
  </script>
</body>
</html>
)rawliteral";

#endif