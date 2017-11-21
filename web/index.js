(function () {
  'use strict';

  document.addEventListener('DOMContentLoaded', event => {
    const connectButton = document.querySelector('#connect');
    const statusDisplay = document.querySelector('#status');
    const logsContainer = document.querySelector('.output-container');
    let port;

    function connect() {
      port.connect().then(() => {
        statusDisplay.textContent = '';
        connectButton.textContent = 'Disconnect';

        port.onReceive = data => {
          let textDecoder = new TextDecoder();
          logsContainer.innerHTML = logsContainer.innerHTML + '<br/>' + textDecoder.decode(data);
        }
        port.onReceiveError = error => {
          console.error(error);
        };
      }, error => {
        statusDisplay.textContent = error;
      });
    }

    function onUpdate(e) {
      if (!port) {
        return;
      }

      let view = new Uint8Array(1);
      view[0] = parseInt(e.charCode);
      port.send(view);
    };

    document.addEventListener('keypress', onUpdate);

    connectButton.addEventListener('click', function () {
      if (port) {
        port.disconnect();
        connectButton.textContent = 'Connect';
        statusDisplay.textContent = '';
        port = null;
      } else {
        serial.requestPort().then(selectedPort => {
          port = selectedPort;
          connect();
        }).catch(error => {
          statusDisplay.textContent = error;
        });
      }
    });

    serial.getPorts().then(ports => {
      if (ports.length == 0) {
        statusDisplay.textContent = 'No device found.';
      } else {
        statusDisplay.textContent = 'Connecting...';
        port = ports[0];
        connect();
      }
    });
  });
})();
