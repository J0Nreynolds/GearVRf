package org.gearvrf.scene_objects;

import android.os.Message;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

import org.gearvrf.GVRAndroidResource;
import org.gearvrf.GVRBillboard;
import org.gearvrf.GVRContext;
import org.gearvrf.GVRCursorController;
import org.gearvrf.GVRMaterial;
import org.gearvrf.GVRMesh;
import org.gearvrf.GVRPhongShader;
import org.gearvrf.GVRPicker;
import org.gearvrf.GVRSceneObject;
import org.gearvrf.GVRTexture;
import org.gearvrf.IPickEvents;
import org.gearvrf.ISensorEvents;
import org.gearvrf.R;
import org.gearvrf.SensorEvent;
import org.joml.Vector3f;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Future;

import static android.util.Config.LOGD;

/**
 * Created by j.reynolds on 6/30/2017.
 */

public class GVRGearControllerSceneObject extends GVRSceneObject {
    private final String TAG = GVRGearControllerSceneObject.class.getSimpleName();
    private float cursorDepth = 1.0f;
    private GVRSceneObject cursor = null;
    private GVRSceneObject controller = null;
    private GVRSceneObject ray = null;
    private GVRPicker picker = null;
    private PickListener pickListener = new PickListener();


    /**
     * Constructor for GVRGearControllerSceneObject
     *
     * This constructor creates a {@link GVRSceneObject} resembling a
     * physical Gear VR Controller and assigns it as a child to this object
     *
     * @param gvrContext    current {@link GVRContext}
     */
    public GVRGearControllerSceneObject(GVRContext gvrContext){
        super(gvrContext);
        controller = new GVRSceneObject(gvrContext, gvrContext.getAssetLoader().loadMesh(new GVRAndroidResource(gvrContext, R.raw.gear_vr_controller)));
        GVRTexture tex = (gvrContext.getAssetLoader().loadTexture(new GVRAndroidResource(gvrContext, R.drawable.gear_vr_controller_color_1024)));
        GVRMaterial mat = new GVRMaterial(gvrContext);
        mat.setMainTexture(tex);
        controller.getRenderData().setMaterial(mat);
        controller.getRenderData().getTransform().setScale(0.05f,0.05f,0.05f);
        controller.getRenderData().getTransform().rotateByAxis(270f, 1, 0, 0);
        controller.getRenderData().getTransform().rotateByAxis(180f, 0, 1,0);
        addChildObject(controller);
        ray = new GVRLineSceneObject(gvrContext, cursorDepth);
        ray.getRenderData().setShaderTemplate(GVRPhongShader.class);
        GVRMaterial rayMaterial = new GVRMaterial(gvrContext);
        rayMaterial.setDiffuseColor(0.5f,0.5f,0.5f,1);
        rayMaterial.setLineWidth(2.0f);
        ray.getRenderData().disableLight();
        ray.getRenderData().setMaterial(rayMaterial);
        addChildObject(ray);
    }

    public void setCursor(GVRSceneObject obj){
        cursor = obj;
        cursor.getTransform().setPosition(0, 0, -cursorDepth);
        cursor.getRenderData().setDepthTest(false);
        cursor.getRenderData().setRenderingOrder(100000);
        addChildObject(cursor);
    }

    public void removeCursor(){
        removeChildObject(cursor);
        cursor = null;
    }

    public GVRSceneObject getRay(){
        return ray;
    }

    public void enableRay(){
        ray.setEnable(true);
    }

    public void disableRay(){
        ray.setEnable(false);
    }

    public void setCursorDepth(float depth){
        this.cursorDepth = Math.abs(depth);
        ray.getTransform().setScaleZ(cursorDepth);
        if(cursor != null)
            cursor.getTransform().setPosition(0, 0, -cursorDepth);
    }

    public float getCursorDepth(){
        return this.cursorDepth;
    }


    public void setControllerObject(GVRSceneObject obj){
        removeChildObject(controller);
        controller = obj;
        addChildObject(controller);
    }

    public GVRSceneObject getControllerObject(){
        return controller;
    }

    public GVRPicker attachPicker(){
        picker = new GVRPicker(this, getGVRContext().getMainScene());
        return picker;
    }

    public void addProjectiveObject(GVRSceneObject obj) {
        if(picker != null)
            obj.getEventReceiver().addListener(pickListener);
    }

    public void removeProjectiveObject(GVRSceneObject obj){
        if(picker != null)
            obj.getEventReceiver().removeListener(pickListener);
    }

    private class PickListener implements IPickEvents {

        private final float[] nullCoords = {-1f, -1f, -1f};

        @Override
        public void onPick(GVRPicker picker){}

        @Override
        public void onNoPick(GVRPicker picker){}

        @Override
        public void onEnter(GVRPicker gvrPicker, GVRSceneObject sceneObj, GVRPicker.GVRPickedObject collision) {
            if(gvrPicker != picker) return;
            if(cursor != null && !Arrays.equals(collision.getBarycentricCoords(), nullCoords)) {
                    removeChildObject(cursor);
                    sceneObj.addChildObject(cursor);
            }
            onInside(gvrPicker, sceneObj, collision);
        }

        @Override
        public void onExit(GVRPicker gvrPicker, GVRSceneObject sceneObj){
            if(gvrPicker != picker) return;
            ray.getTransform().setScaleZ(cursorDepth);
            if(cursor != null) {
                sceneObj.removeChildObject(cursor);
                cursor.getTransform().reset();
                cursor.getTransform().setPosition(0, 0, -cursorDepth);
                addChildObject(cursor);
            }
        }

        @Override
        public void onInside(GVRPicker gvrPicker, GVRSceneObject sceneObj, GVRPicker.GVRPickedObject collision){
            if(gvrPicker != picker) return;
            ray.getTransform().setScaleZ(collision.hitDistance);
            if(cursor != null && !Arrays.equals(collision.getBarycentricCoords(), nullCoords)) {
                Vector3f lookat = new Vector3f(0, 0, 0);
                Vector3f up = new Vector3f(0, 1, 0);
                Vector3f Xaxis = new Vector3f(0, 0, 0);
                Vector3f Yaxis = new Vector3f(0, 0, 0);

                lookat.set(collision.getNormalX(), collision.getNormalY(), collision.getNormalZ());
                lookat = lookat.normalize();

                up.cross(lookat.x, lookat.y, lookat.z, Xaxis);
                Xaxis = Xaxis.normalize();

                lookat.cross(Xaxis.x, Xaxis.y, Xaxis.z, Yaxis);
                Yaxis = Yaxis.normalize();

                cursor.getTransform().setModelMatrix(new float[]{Xaxis.x, Xaxis.y, Xaxis.z, 0.0f,
                        Yaxis.x, Yaxis.y, Yaxis.z, 0.0f,
                        lookat.x, lookat.y, lookat.z, 0.0f,
                        collision.getHitX(), +collision.getHitY(), collision.getHitZ(), 1.0f});
            }
        }
    };

}
