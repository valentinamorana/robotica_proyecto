package com.example.ledcontroller;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import java.io.IOException;
import java.io.OutputStream;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    // --- Config Bluetooth ---
    private static final String DEVICE_ADDRESS = "58:56:00:00:CD:7E"; // MAC HC-05
    private static final UUID MY_UUID =
            UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private static final int REQ_BT_CONNECT = 1001;

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private OutputStream outputStream;

    // --- UI ---
    private Button connectBtn, allBtn, leftBtn, rightBtn, centerBtn;
    private Switch soundSwitch;
    private SeekBar brightnessSeek;
    private TextView statusText;

    // Colores
    private Button redBtn, greenBtn, blueBtn, whiteBtn, rainbowBtn;
    private Button yellowBtn, cyanBtn, magentaBtn, pinkBtn;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // BT
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        // UI bindings
        connectBtn = findViewById(R.id.connectBtn);
        soundSwitch = findViewById(R.id.soundSwitch);
        brightnessSeek = findViewById(R.id.brightnessSeek);
        allBtn = findViewById(R.id.allBtn);
        leftBtn = findViewById(R.id.leftBtn);
        rightBtn = findViewById(R.id.rightBtn);
        centerBtn = findViewById(R.id.centerBtn);
        statusText = findViewById(R.id.statusText);

        redBtn = findViewById(R.id.redBtn);
        greenBtn = findViewById(R.id.greenBtn);
        blueBtn = findViewById(R.id.blueBtn);
        whiteBtn = findViewById(R.id.whiteBtn);
        rainbowBtn = findViewById(R.id.rainbowBtn);
        yellowBtn = findViewById(R.id.yellowBtn);
        cyanBtn = findViewById(R.id.cyanBtn);
        magentaBtn = findViewById(R.id.magentaBtn);
        pinkBtn = findViewById(R.id.pinkBtn);

        // Listeners
        connectBtn.setOnClickListener(v -> connectToHC05());

        soundSwitch.setOnCheckedChangeListener((b, checked) ->
                sendCommand(checked ? "SOUND_ON" : "SOUND_OFF"));

        brightnessSeek.setMax(255);
        brightnessSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) sendCommand("BRIGHT:" + progress);
            }
            @Override public void onStartTrackingTouch(SeekBar seekBar) { }
            @Override public void onStopTrackingTouch(SeekBar seekBar) { }
        });

        allBtn.setOnClickListener(v -> sendCommand("ALL"));
        leftBtn.setOnClickListener(v -> sendCommand("LEFT"));
        rightBtn.setOnClickListener(v -> sendCommand("RIGHT"));
        centerBtn.setOnClickListener(v -> sendCommand("CENTER"));

        redBtn.setOnClickListener(v -> sendCommand("COLOR:RED"));
        greenBtn.setOnClickListener(v -> sendCommand("COLOR:GREEN"));
        blueBtn.setOnClickListener(v -> sendCommand("COLOR:BLUE"));
        whiteBtn.setOnClickListener(v -> sendCommand("COLOR:WHITE"));
        rainbowBtn.setOnClickListener(v -> sendCommand("RAINBOW"));
        rainbowBtn.setOnLongClickListener(v -> { sendCommand("RAINBOW DIR"); return true; });
        yellowBtn.setOnClickListener(v -> sendCommand("COLOR:YELLOW"));
        cyanBtn.setOnClickListener(v -> sendCommand("COLOR:CYAN"));
        magentaBtn.setOnClickListener(v -> sendCommand("COLOR:MAGENTA"));
        pinkBtn.setOnClickListener(v -> sendCommand("COLOR:PINK"));
    }

    // ---- Permiso BLUETOOTH_CONNECT (Android 12+) ----
    private boolean ensureBtConnectPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
                    != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(
                        this,
                        new String[]{Manifest.permission.BLUETOOTH_CONNECT},
                        REQ_BT_CONNECT
                );
                return false;
            }
        }
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQ_BT_CONNECT) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                connectToHC05();
            } else {
                toast("Permiso Bluetooth denegado");
            }
        }
    }

    // ---- Conexión a HC-05 por MAC ----
    private void connectToHC05() {
        if (bluetoothAdapter == null) {
            toast("Bluetooth no disponible");
            return;
        }
        if (!bluetoothAdapter.isEnabled()) {
            toast("Activa el Bluetooth");
            return;
        }
        if (!ensureBtConnectPermission()) return;

        try {
            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(DEVICE_ADDRESS);
            bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
            bluetoothSocket.connect();
            outputStream = bluetoothSocket.getOutputStream();
            setStatus("Conectado a HC-05");
            toast("Conectado a HC-05");
        } catch (SecurityException se) {
            setStatus("Falta permiso BLUETOOTH_CONNECT");
            toast("Falta permiso BLUETOOTH_CONNECT");
        } catch (IOException e) {
            setStatus("Error al conectar");
            toast("Error al conectar: " + e.getMessage());
        }
    }

    // ---- Envío de comandos ----
    private void sendCommand(String cmd) {
        if (outputStream == null) {
            setStatus("Desconectado");
            toast("No conectado");
            return;
        }
        try {
            outputStream.write((cmd + "\n").getBytes());
            // toast("→ " + cmd); // (opcional: feedback)
        } catch (IOException e) {
            setStatus("Desconectado");
            toast("Error al enviar");
        }
    }

    private void setStatus(String s) {
        if (statusText != null) statusText.setText("Estado: " + s);
    }

    private void toast(String s) {
        Toast.makeText(this, s, Toast.LENGTH_SHORT).show();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try { if (outputStream != null) outputStream.close(); } catch (IOException ignored) {}
        try { if (bluetoothSocket != null) bluetoothSocket.close(); } catch (IOException ignored) {}
    }
}
