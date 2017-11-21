(function () {
  'use strict';

  document.addEventListener('DOMContentLoaded', async event => {
    const connectButton = document.querySelector('#connect');
    const statusDisplay = document.querySelector('#status');
    let port;

    // Connect to the selected USB device and setup receive hooks
    async function connect() {
      try {
        await port.connect();
        statusDisplay.textContent = '';
        connectButton.textContent = 'Disconnect';

        port.onReceive = data => {
          let textDecoder = new TextDecoder();
          log(textDecoder.decode(data), port.device_.productName);
        }
        port.onReceiveError = error => {
          console.error(error);
        };
      } catch (error) {
        statusDisplay.textContent = error;
      }
    }

    // Send data to USB device
    function sendData(key) {
      if (!port) {
        return;
      }

      const view = new Uint8Array(1);
      view[0] = parseInt(key);
      port.send(view);
    };

    // Attach listeners
    document.addEventListener('keypress', event => sendData(event.charCode));
    connectButton.addEventListener('click', async function () {
      if (port) {
        port.disconnect();
        connectButton.textContent = 'Connect';
        statusDisplay.textContent = '';
        port = null;
      } else {
        try {
          port = await serial.requestPort();
          connect();
        } catch (error) {
          statusDisplay.textContent = error;
        }
      }
    });
    document.querySelector('.up').addEventListener('click', () => sendData('w'.charCodeAt(0)));
    document.querySelector('.down').addEventListener('click', () => sendData('s'.charCodeAt(0)));
    document.querySelector('.left').addEventListener('click', () => sendData('a'.charCodeAt(0)));
    document.querySelector('.right').addEventListener('click', () => sendData('d'.charCodeAt(0)));

    // Log data to page
    function log(data, source) {
      const logsContainer = document.querySelector('.output-container');
      const logItem = document.importNode(document.querySelector('#log-item-template').content, true);
      logItem.querySelector('.time').textContent = `[${new Date()}]`;
      logItem.querySelector('.source').textContent = `[${source}]`;
      logItem.querySelector('.data').textContent = data;
      logsContainer.appendChild(logItem);
      logsContainer.scrollTop = logsContainer.scrollHeight;
    }

    // Show connectivity status
    const ports = await serial.getPorts();
    if (ports.length == 0) {
      statusDisplay.textContent = 'No device found.';
    } else {
      statusDisplay.textContent = 'Connecting...';
      port = ports[0];
      connect();
    }
  });
})();
