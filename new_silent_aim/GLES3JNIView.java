package com.mycompany.application;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * GLES3JNIView - OpenGL ES 3.0 View com JNI
 * Interface entre Java e código nativo C++
 */
public class GLES3JNIView extends GLSurfaceView implements Renderer {

    public GLES3JNIView(Context context) {
        super(context);

        // Configurar versão OpenGL ES 3.0
        setEGLContextClientVersion(3);

        // Configurar atributos EGL
        setEGLConfigChooser(8, 8, 8, 8, 16, 0);

        // Definir este renderer
        setRenderer(this);

        // Renderizar continuamente
        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        // Chamada nativa para inicializar
        init();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        // Chamada nativa para resize
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        // Chamada nativa para renderizar frame
        step();
    }

    /**
     * Métodos JNI - Implementados na biblioteca nativa (libmatveuxxx.so)
     */

    /**
     * Inicializa OpenGL ES e Silent Aim
     */
    public native void init();

    /**
     * Renderiza um frame
     */
    public native void step();

    /**
     * Ajusta o viewport quando a tela muda de tamanho
     */
    public native void resize(int width, int height);

    /**
     * Processa eventos de toque/mouse
     */
    public native void MotionEventClick(float x, float y, int action);

    /**
     * Obtém dimensões da janela
     */
    public native int[] getWindowRect();

    /**
     * Cleanup de recursos
     */
    public native void imgui_Shutdown();

    /**
     * Carregar biblioteca nativa
     */
    static {
        System.loadLibrary("matveuxxx");
    }
}
