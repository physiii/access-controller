This folder is mounted into the tunnel container at `/app/device-ui`.

When populated with `index.html`, `style.css`, and `script.js`, the tunnel server
will serve those assets directly at:

- `GET /device/<deviceId>/` (index)
- `GET /device/<deviceId>/style.css`
- `GET /device/<deviceId>/script.js`

This avoids fetching UI assets through the ESP32 tunnel, which can time out or
exhaust heap on the controller during page load.

To populate it from the firmware UI sources in this repo:

```bash
cp -v ../controller/main/public/index.html ./device-ui/index.html
cp -v ../controller/main/public/style.css ./device-ui/style.css
cp -v ../controller/main/public/script.js ./device-ui/script.js
```

