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
import org.gearvrf.GVRPicker;
import org.gearvrf.GVRSceneObject;
import org.gearvrf.GVRTexture;
import org.gearvrf.IPickEvents;
import org.gearvrf.R;
import org.joml.Vector3f;

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
    private PickListener pickListener = new PickListener();


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
    }

    public void setCursorTexture(Future<GVRTexture> texture){
        GVRContext context = getGVRContext();
        cursor = new GVRSceneObject(context, 0.5f, 0.5f);
        GVRMaterial mat = new GVRMaterial(context);
        mat.setMainTexture(texture);
        cursor.getRenderData().setMaterial(mat);
        cursor.getRenderData().getTransform().setPosition(0, 0, -cursorDepth);
        cursor.getRenderData().setDepthTest(false);
        cursor.getRenderData().setRenderingOrder(100000);
        addChildObject(cursor);
    }

    public void setCursorDepth(float depth){
        this.cursorDepth = Math.abs(depth);
        cursor.getRenderData().getTransform().setPosition(0, 0, -cursorDepth);
    }

    public float getCursorDepth(){
        return this.cursorDepth;
    }

    public void setMesh(GVRMesh mesh){
        controller.getRenderData().setMesh(mesh);
        controller.getRenderData().setMaterial(new GVRMaterial(getGVRContext()));
    }


    public void setControllerObject(GVRSceneObject obj){
        removeChildObject(controller);
        controller = obj;
        addChildObject(controller);
    }

    public GVRSceneObject getControllerObject(){
        return controller;
    }

    public void addProjectiveObject(GVRSceneObject obj) {
        obj.getEventReceiver().addListener(pickListener);
    }

    public void removeProjectiveObject(GVRSceneObject obj){
        obj.getEventReceiver().removeListener(pickListener);
    }

    private class PickListener implements IPickEvents {

        @Override
        public void onPick(GVRPicker picker){}

        @Override
        public void onNoPick(GVRPicker picker){}

        @Override
        public void onEnter(GVRSceneObject sceneObj, GVRPicker.GVRPickedObject collision) {
            removeChildObject(cursor);
            sceneObj.addChildObject(cursor);
        }

        @Override
        public void onExit(GVRSceneObject sceneObj){
            sceneObj.removeChildObject(cursor);
            cursor.getTransform().reset();
            cursor.getTransform().setPosition(0, 0, -cursorDepth);
            addChildObject(cursor);
        }

        @Override
        public void onInside(GVRSceneObject sceneObj, GVRPicker.GVRPickedObject collision){
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
                    collision.getHitX(), + collision.getHitY(), collision.getHitZ(), 1.0f});
        }
    };

}
