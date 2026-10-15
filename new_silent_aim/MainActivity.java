package com.mycompany.application;

import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.WindowManager;
import android.opengl.GLSurfaceView;

/**
 * MainActivity - Aplicação de Silent Aim
 * Controlador principal da aplicação Android
 */
public class MainActivity extends Activity {

    private GLES3JNIView glView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Carrega a biblioteca nativa
        System.loadLibrary("matveuxxx");

        // Criar GLSurfaceView
        glView = new GLES3JNIView(this);
        glView.setOnTouchListener((v, event) -> {
            float x = event.getX();
            float y = event.getY();
            int action = event.getActionMasked();

            // Passar evento de toque para a biblioteca nativa
            glView.MotionEventClick(x, y, action);
            return true;
        });

        setContentView(glView);

        // Manter tela ligada
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (glView != null) {
            glView.onPause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (glView != null) {
            glView.onResume();
        }
    }

    @Override
    protected void onDestroy() {
        if (glView != null) {
            glView.imgui_Shutdown();
        }
        super.onDestroy();
    }
}
