package org.gearvrf.scene_objects;

import org.gearvrf.GVRContext;
import org.gearvrf.GVRMesh;
import org.gearvrf.GVRSceneObject;

/**
 * Created by j.reynolds on 7/10/2017.
 */

public class GVRLineSceneObject extends GVRSceneObject {

    public GVRLineSceneObject(GVRContext gvrContext){
        this(gvrContext, 1.0f);
    }

    public GVRLineSceneObject(GVRContext gvrContext, float length){
        super(gvrContext, generateLine(gvrContext, length));
        this.getRenderData().setDrawMode(android.opengl.GLES30.GL_LINES);
    }

    private static GVRMesh generateLine(GVRContext gvrContext, float length){
        GVRMesh mesh = new GVRMesh(gvrContext);
        float[] vertices = {
                0,          0,          0,
                0,          0,          -length
        };
        mesh.setVertices(vertices);
        return mesh;
    }
}
