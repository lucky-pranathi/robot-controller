package com.lucky.robotcontroller;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    // Change this to match the name your ESP32 advertises
    private static final String TARGET_DEVICE_NAME = "ESP32_FaceRobot";

    // Standard SPP UUID (works with ESP32 BluetoothSerial library)
    private static final UUID SPP_UUID =
            UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private static final int REQUEST_PERMISSIONS_CODE = 100;

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private OutputStream outputStream;

    // Guards against double-connect attempts and lets sendData() know the real state
    private volatile boolean isConnecting = false;
    private volatile boolean isConnected = false;

    private TextView tvStatus;
    private EditText etMessage;
    private Button btnConnect, btnSend;
    ImageButton refreshBtn;
    private ImageButton forward, back, left, right;

    private final Handler mainHandler = new Handler(Looper.getMainLooper());

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvStatus = findViewById(R.id.tvStatus);
        refreshBtn=findViewById(R.id.refreshBtn);
        etMessage = findViewById(R.id.etMessage);
        btnConnect = findViewById(R.id.btnConnect);
        btnSend = findViewById(R.id.btnSend);
        forward = findViewById(R.id.forward);
        back = findViewById(R.id.back);
        left = findViewById(R.id.left);
        right = findViewById(R.id.right);

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth not supported on this device", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        // Each directional button gets ONE touch listener that both animates
        // the button AND sends the matching command. (Previously a second
        // setOnTouchListener() call was silently overwriting the first one
        // set up by addPressEffect(), so the animation listener was dead code.)
        setupDirectionButton(forward, "f");
        setupDirectionButton(back, "b");
        setupDirectionButton(left, "l");
        setupDirectionButton(right, "r");

        btnConnect.setOnClickListener(v -> checkPermissionsAndConnect());
        btnSend.setOnClickListener(v -> sendMessage());

        refreshBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MainActivity.class);
                startActivity(intent);
                finish();
            }
        });
    }

    private void setupDirectionButton(ImageButton button, String command) {
        button.setOnTouchListener((v, event) -> {
            handlePressAnimation(v, event);

            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    sendData(command);
                    break;

                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    sendData("s");
                    break;
            }
            return true;
        });
    }

    private void checkPermissionsAndConnect() {
        if (!hasBluetoothPermissions()) {
            String[] permissions;
            if (android.os.Build.VERSION.SDK_INT >= 31) {
                permissions = new String[]{
                        Manifest.permission.BLUETOOTH_CONNECT,
                        Manifest.permission.BLUETOOTH_SCAN
                };
            } else {
                permissions = new String[]{
                        Manifest.permission.ACCESS_FINE_LOCATION
                };
            }
            ActivityCompat.requestPermissions(this, permissions, REQUEST_PERMISSIONS_CODE);
            return;
        }
        connectToDeviceByName();
    }

    private boolean hasBluetoothPermissions() {
        if (android.os.Build.VERSION.SDK_INT >= 31) {
            return ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
                    == PackageManager.PERMISSION_GRANTED;
        } else {
            return ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION)
                    == PackageManager.PERMISSION_GRANTED;
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSIONS_CODE) {
            boolean granted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    granted = false;
                    break;
                }
            }
            if (granted) {
                connectToDeviceByName();
            } else {
                Toast.makeText(this, "Bluetooth permissions are required", Toast.LENGTH_LONG).show();
            }
        }
    }

    // Looks through already-paired devices and connects to the one matching TARGET_DEVICE_NAME
    private void connectToDeviceByName() {
        if (!hasBluetoothPermissions()) {
            Toast.makeText(this, "Bluetooth permission missing", Toast.LENGTH_SHORT).show();
            return;
        }

        if (isConnecting || isConnected) {
            Toast.makeText(this, "Already connecting/connected", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!bluetoothAdapter.isEnabled()) {
            Toast.makeText(this, "Please enable Bluetooth", Toast.LENGTH_SHORT).show();
            return;
        }

        BluetoothDevice targetDevice = null;

        try {
            Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
            for (BluetoothDevice device : pairedDevices) {
                if (device.getName() != null && device.getName().equals(TARGET_DEVICE_NAME)) {
                    targetDevice = device;
                    break;
                }
            }
        } catch (SecurityException e) {
            Toast.makeText(this, "Bluetooth permission missing", Toast.LENGTH_SHORT).show();
            return;
        }

        if (targetDevice == null) {
            updateStatus("Status: " + TARGET_DEVICE_NAME + " not found in paired devices");
            Toast.makeText(this, "Pair with " + TARGET_DEVICE_NAME + " first in phone settings", Toast.LENGTH_LONG).show();
            return;
        }

        final BluetoothDevice device = targetDevice;
        isConnecting = true;
        updateStatus("Status: Connecting to " + device.getName() + "...");

        new Thread(() -> {
            try {
                bluetoothSocket = device.createRfcommSocketToServiceRecord(SPP_UUID);
                bluetoothAdapter.cancelDiscovery();
                bluetoothSocket.connect();
                outputStream = bluetoothSocket.getOutputStream();
                isConnected = true;

                mainHandler.post(() -> updateStatus("Status: Connected to " + device.getName()));
            } catch (IOException | SecurityException e) {
                isConnected = false;
                mainHandler.post(() -> updateStatus("Status: Connection failed"));
                try {
                    if (bluetoothSocket != null) bluetoothSocket.close();
                } catch (IOException closeEx) {
                    closeEx.printStackTrace();
                }
            } finally {
                isConnecting = false;
            }
        }).start();
    }

    private void sendMessage() {
        String message = etMessage.getText().toString();
        if (message.isEmpty()) return;

        sendData(message);
        etMessage.setText("");
    }

    private void updateStatus(String text) {
        tvStatus.setText(text);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        isConnected = false;
        try {
            if (outputStream != null) outputStream.close();
            if (bluetoothSocket != null) bluetoothSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void handlePressAnimation(View v, MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            v.animate().scaleX(0.85f).scaleY(0.85f).setDuration(120).start();
        } else if (event.getAction() == MotionEvent.ACTION_UP ||
                event.getAction() == MotionEvent.ACTION_CANCEL) {
            v.animate().scaleX(1f).scaleY(1f).setDuration(120).start();
        }
    }

    // ---------------------------------------------------------
    // SAFE BLUETOOTH SEND — now uses the SAME socket that
    // connectToDeviceByName() opens, instead of a disconnected
    // BluetoothConnectionManager singleton.
    // ---------------------------------------------------------
    private void sendData(String msg) {
        if (!isConnected || outputStream == null) {
            Toast.makeText(this, "Not connected to ESP32", Toast.LENGTH_SHORT).show();
            return;
        }

        new Thread(() -> {
            try {
                outputStream.write((msg + "\n").getBytes());
                outputStream.flush();
            } catch (IOException e) {
                isConnected = false;
                mainHandler.post(() -> Toast.makeText(this, "Send failed", Toast.LENGTH_SHORT).show());
            }
        }).start();
    }
}