#include "ofxManipulator.h"

ofxManipulator::ofxManipulator()
:m_currScale              (SCALE_NONE)
,m_currScalePredict       (SCALE_NONE)
,m_currRotation           (ROTATION_NONE)
,m_currRotationPredict    (ROTATION_NONE)
,m_currTranslation        (TRANSLATION_NONE)
,m_currTranslationPredict (TRANSLATION_NONE)
,m_currManipulator        (MANIPULATOR_NONE)
,m_x_color                (0xff, 0x41, 0x36)
,m_y_color                (0x2e, 0xcc, 0x40)
,m_z_color                (0x00, 0x74, 0xd9)
,m_w_color                (0xff, 0x4b, 0xff)
,m_select_color           (0xff, 0xdc, 0x00)
,m_scale                  (1.0f, 1.0f, 1.0f)
,m_manipulatorScale       (1.0f)
,m_screenFactor           (0.0f)
,m_angleRad               (0.0f)
{
  auto &events = ofEvents();
  ofAddListener(events.mouseMoved   , this, &ofxManipulator::mouseMoved   , OF_EVENT_ORDER_BEFORE_APP);
  ofAddListener(events.mousePressed , this, &ofxManipulator::mousePressed , OF_EVENT_ORDER_BEFORE_APP);
  ofAddListener(events.mouseDragged , this, &ofxManipulator::mouseDragged , OF_EVENT_ORDER_BEFORE_APP);
  ofAddListener(events.mouseReleased, this, &ofxManipulator::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
}

ofxManipulator::~ofxManipulator()
{
  auto &events = ofEvents();
  ofRemoveListener(events.mouseMoved   , this, &ofxManipulator::mouseMoved   , OF_EVENT_ORDER_BEFORE_APP);
  ofRemoveListener(events.mousePressed , this, &ofxManipulator::mousePressed , OF_EVENT_ORDER_BEFORE_APP);
  ofRemoveListener(events.mouseDragged , this, &ofxManipulator::mouseDragged , OF_EVENT_ORDER_BEFORE_APP);
  ofRemoveListener(events.mouseReleased, this, &ofxManipulator::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void ofxManipulator::draw(ofCamera cam)
{
  ofPushStyle();

  ofFill();
  ofDisableDepthTest();

  m_view = cam.getModelViewMatrix();
  m_proj = cam.getProjectionMatrix();
  m_viewInverse = m_view.getInverse();
  m_screenFactor = computeScreenFactor();

  ofVec3f origin = m_translation;
  float screenf = m_screenFactor;
  float screenf_half = screenf * 0.5f;

  ofMatrix4x4 m;
  m.rotate(m_rotation);
  ofVec3f axe_x = m.getRowAsVec3f(0);
  ofVec3f axe_y = m.getRowAsVec3f(1);
  ofVec3f axe_z = m.getRowAsVec3f(2);

  ofColor unit_color_x = m_x_color;
  ofColor unit_color_y = m_y_color;
  ofColor unit_color_z = m_z_color;
  ofColor unit_color_w = m_w_color;
  ofColor select_color = m_select_color;

  switch (m_currManipulator)
  {
    case (MANIPULATOR_SCALE):
    {
      SCALE_TYPE curr = m_currScalePredict;

      drawTriangle(origin, screenf_half, ( (curr == SCALE_XZ) || (curr == SCALE_XYZ) ), axe_x, axe_z, unit_color_y);
      drawTriangle(origin, screenf_half, ( (curr == SCALE_XY) || (curr == SCALE_XYZ) ), axe_x, axe_y, unit_color_z);
      drawTriangle(origin, screenf_half, ( (curr == SCALE_YZ) || (curr == SCALE_XYZ) ), axe_y, axe_z, unit_color_x);

      axe_x *= screenf;
      axe_y *= screenf;
      axe_z *= screenf;

      drawScaleAxis(origin, axe_x, axe_y, ( (curr == SCALE_X) || (curr == SCALE_XYZ) ) ? select_color : unit_color_x);
      drawScaleAxis(origin, axe_y, axe_x, ( (curr == SCALE_Y) || (curr == SCALE_XYZ) ) ? select_color : unit_color_y);
      drawScaleAxis(origin, axe_z, axe_x, ( (curr == SCALE_Z) || (curr == SCALE_XYZ) ) ? select_color : unit_color_z);
    }
    break;
    case (MANIPULATOR_ROTATION):
    {
      ROTATION_TYPE type = m_currRotation;
      ROTATION_TYPE curr = m_currRotationPredict;

      ofVec3f right;
      ofVec3f front;
      ofVec3f dir;
      ofVec3f up;

      dir = origin - m_viewInverse.getTranslation();
      dir.normalize();

      right = dir;
      right.cross(axe_y);
      right.normalize();

      up = dir;
      up.cross(right);
      up.normalize();

      right = dir;
      right.cross(up);
      right.normalize();

      {
        ofVec3f vtx = 1.2f * screenf * up;
        ofVec3f vty = 1.2f * screenf * right;
        drawCircle(origin, vtx, vty, curr == ROTATION_SCREEN ? select_color : unit_color_w);
      }

      {
        right = dir;
        right.cross(axe_x);
        right.normalize();
        front = right;
        front.cross(axe_x);
        front.normalize();
        ofVec3f vtx = screenf * right;
        ofVec3f vty = screenf * front;
        drawCircle(origin, vtx, vty, curr == ROTATION_X ? select_color : unit_color_x);
      }

      {
        right = dir;
        right.cross(axe_y);
        right.normalize();
        front = right;
        front.cross(axe_y);
        front.normalize();
        ofVec3f vtx = screenf * right;
        ofVec3f vty = screenf * front;
        drawCircle(origin, vtx, vty, curr == ROTATION_Y ? select_color : unit_color_y);
      }

      {
        right = dir;
        right.cross(axe_z);
        right.normalize();
        front = right;
        front.cross(axe_z);
        front.normalize();
        ofVec3f vtx = screenf * right;
        ofVec3f vty = screenf * front;
        drawCircle(origin, vtx, vty, curr == ROTATION_Z ? select_color : unit_color_z);
      }

      if (type != ROTATION_NONE)
      {
        ofVec3f x = m_rotate_x;
        ofVec3f y = m_rotate_y;
        float angle = m_angleRad;
        x *= screenf;
        y *= screenf;
        switch (curr)
        {
          case (ROTATION_X):
            drawCamembert(origin, x, y, -angle, unit_color_x);
            break;
          case (ROTATION_Y):
            drawCamembert(origin, x, y, -angle, unit_color_y);
            break;
          case (ROTATION_Z):
            drawCamembert(origin, x, y, -angle, unit_color_z);
            break;
          case (ROTATION_SCREEN):
            drawCamembert(origin, x, y, -angle, unit_color_w);
            break;
        }
      }
    }
    break;
    case (MANIPULATOR_TRANSLATION):
    {
      TRANSLATION_TYPE curr = m_currTranslationPredict;

      drawQuad(origin, screenf_half, ( (curr == TRANSLATION_XZ) || (curr == TRANSLATION_XYZ) ), axe_x, axe_z, unit_color_y);
      drawQuad(origin, screenf_half, ( (curr == TRANSLATION_XY) || (curr == TRANSLATION_XYZ) ), axe_x, axe_y, unit_color_z);
      drawQuad(origin, screenf_half, ( (curr == TRANSLATION_YZ) || (curr == TRANSLATION_XYZ) ), axe_y, axe_z, unit_color_x);

      axe_x *= screenf;
      axe_y *= screenf;
      axe_z *= screenf;

      drawAxis(origin, axe_x, axe_y, axe_z, 0.05f, 0.83f, ( (curr == TRANSLATION_X) || (curr == TRANSLATION_XYZ) ) ? select_color : unit_color_x);
      drawAxis(origin, axe_y, axe_x, axe_z, 0.05f, 0.83f, ( (curr == TRANSLATION_Y) || (curr == TRANSLATION_XYZ) ) ? select_color : unit_color_y);
      drawAxis(origin, axe_z, axe_x, axe_y, 0.05f, 0.83f, ( (curr == TRANSLATION_Z) || (curr == TRANSLATION_XYZ) ) ? select_color : unit_color_z);
    }
    break;
  }

  ofEnableDepthTest();

  ofPopStyle();
}

ofMatrix4x4 ofxManipulator::getMatrix()
{
  ofMatrix4x4 m;
  m.scale(m_scale);
  m.rotate(m_rotation);
  m.translate(m_translation);
  return m;
}

ofVec3f ofxManipulator::getScale()
{
  return m_scale;
}

ofQuaternion ofxManipulator::getRotation()
{
  return m_rotation;
}

ofVec3f ofxManipulator::getTranslation()
{
  return m_translation;
}

void ofxManipulator::setScale(ofVec3f scale)
{
  m_scale = scale;
}

void ofxManipulator::setRotation(ofQuaternion rotation)
{
  m_rotation = rotation;
}

void ofxManipulator::setTranslation(ofVec3f translation)
{
  m_translation = translation;
}

void ofxManipulator::scale(ofVec3f scale)
{
  m_scale *= scale;
}

void ofxManipulator::rotate(ofQuaternion rotation)
{
  m_rotation *= rotation;
}

void ofxManipulator::translate(ofVec3f translation)
{
  m_translation += translation;
}

void ofxManipulator::toggleScale()
{
  m_currManipulator = (m_currManipulator == MANIPULATOR_SCALE) ? MANIPULATOR_NONE : MANIPULATOR_SCALE;
}

void ofxManipulator::toggleRotation()
{
  m_currManipulator = (m_currManipulator == MANIPULATOR_ROTATION) ? MANIPULATOR_NONE : MANIPULATOR_ROTATION;
}

void ofxManipulator::toggleTranslation()
{
  m_currManipulator = (m_currManipulator == MANIPULATOR_TRANSLATION) ? MANIPULATOR_NONE : MANIPULATOR_TRANSLATION;
}

float ofxManipulator::getManipulatorScale()
{
  return m_manipulatorScale;
}

void ofxManipulator::setManipulatorScale(float scale)
{
  m_manipulatorScale = scale;
}

ofxManipulator::MANIPULATOR_TYPE ofxManipulator::getManipulatorType()
{
  return m_currManipulator;
}

void ofxManipulator::setManipulatorType(MANIPULATOR_TYPE type)
{
  m_currManipulator = type;
}

void ofxManipulator::setManipulatorColors(ofColor x, ofColor y, ofColor z, ofColor w, ofColor select)
{
  m_x_color = x;
  m_y_color = y;
  m_z_color = z;
  m_w_color = w;
  m_select_color = select;
}

////////////////////////////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void ofxManipulator::mouseMoved(ofMouseEventArgs &mouse)
{
  int x = mouse.x;
  int y = mouse.y;

  switch (m_currManipulator)
  {
    case (MANIPULATOR_SCALE):
      getCurrScale(m_currScalePredict, x, y);
      break;
    case (MANIPULATOR_ROTATION):
      getCurrRotation(m_currRotationPredict, x, y);
      break;
    case (MANIPULATOR_TRANSLATION):
      getCurrTranslation(m_currTranslationPredict, x, y);
      break;
  }
}

void ofxManipulator::mousePressed(ofMouseEventArgs &mouse)
{
  int x = mouse.x;
  int y = mouse.y;
  int button = mouse.button;

  if(button != OF_MOUSE_BUTTON_LEFT)
    return;

  switch (m_currManipulator)
  {
    case (MANIPULATOR_SCALE):
      getCurrScale(m_currScale, x, y);
      break;
    case (MANIPULATOR_ROTATION):
      getCurrRotation(m_currRotation, x, y);
      break;
    case (MANIPULATOR_TRANSLATION):
      getCurrTranslation(m_currTranslation, x, y);
      break;
  }
}

void ofxManipulator::mouseDragged(ofMouseEventArgs &mouse)
{
  int x = mouse.x;
  int y = mouse.y;
  int button = mouse.button;

  if (button != OF_MOUSE_BUTTON_LEFT)
    return;

  switch (m_currManipulator)
  {
    case (MANIPULATOR_SCALE):
    {
      if (m_currScale == SCALE_NONE) {
        getCurrScale(m_currScalePredict, x, y);
        break;
      }

      ofVec3f ray_origin;
      ofVec3f ray_direction;
      ofVec3f intersection;

      createRay(x, y, ray_origin, ray_direction);
      intersectRay(m_plane, intersection, ray_origin, ray_direction);

      ofMatrix4x4 m;
      m.rotate(m_rotation);
      ofVec3f axe_x = m.getRowAsVec3f(0);
      ofVec3f axe_y = m.getRowAsVec3f(1);
      ofVec3f axe_z = m.getRowAsVec3f(2);

      ofVec3f df;

      if (m_currScale == SCALE_XYZ)
      {
        float scale = x - m_lockedCursor_x;
        df = ofVec3f(scale);
      } else {
        df = intersection - m_lockedPoint;
        switch (m_currScale)
        {
          case SCALE_X:  df = ofVec3f(df.dot(axe_x),0,0); break;
          case SCALE_Y:  df = ofVec3f(0,df.dot(axe_y),0); break;
          case SCALE_Z:  df = ofVec3f(0,0,df.dot(axe_z)); break;
          case SCALE_XY: df = ofVec3f(df.dot(axe_x+axe_y),df.dot(axe_x+axe_y),0); break;
          case SCALE_XZ: df = ofVec3f(df.dot(axe_x+axe_z),0,df.dot(axe_x+axe_z)); break;
          case SCALE_YZ: df = ofVec3f(0,df.dot(axe_y+axe_z),df.dot(axe_y+axe_z)); break;
        }
      }

      float len = (sqrtf(df.x * df.x + df.y * df.y + df.z * df.z)) / 100.f;

      m_scale = m_scaleSaved + (df.normalize() * len);
    }
    break;
    case (MANIPULATOR_ROTATION):
    {
      if (m_currRotation == ROTATION_NONE) {
        getCurrRotation(m_currRotationPredict, x, y);
        break;
      }

      ofVec3f ray_origin;
      ofVec3f ray_direction;
      ofVec3f intersection;

      createRay(x, y, ray_origin, ray_direction);
      intersectRay(m_plane, intersection, ray_origin, ray_direction);

      ofVec3f df = intersection - m_translation;
      df.normalize();

      float acos_angle = df.dot(m_lockedPoint);
      m_angleRad = (acos_angle < -0.99999f) || (acos_angle > 0.99999f) ? 0.0f : acos(acos_angle);

      if (df.dot(m_rotate_y) > 0)
        m_angleRad = -m_angleRad;

      ofMatrix4x4 m;
      m.rotate(m_rotationSaved);
      m.rotateRad(m_angleRad, m_axis.x, m_axis.y, m_axis.z);

      m_rotation = m.getRotate();
    }
    break;
    case (MANIPULATOR_TRANSLATION):
    {
      if (m_currTranslation == TRANSLATION_NONE) {
        getCurrTranslation(m_currTranslationPredict, x, y);
        break;
      }

      ofVec3f ray_origin;
      ofVec3f ray_direction;
      ofVec3f intersection;

      createRay(x, y, ray_origin, ray_direction);
      intersectRay(m_plane, intersection, ray_origin, ray_direction);

      ofMatrix4x4 m;
      m.rotate(m_rotation);
      ofVec3f axe_x = m.getRowAsVec3f(0);
      ofVec3f axe_y = m.getRowAsVec3f(1);
      ofVec3f axe_z = m.getRowAsVec3f(2);

      if (m_currTranslation == TRANSLATION_XYZ)
      {
        ofCamera cam;
        cam.setTransformMatrix(m_viewInverse);
        ofVec3f mat_pos = m_translationSaved;
        ofVec3f cam_pos = cam.getPosition();
        ofVec3f cam_n = cam.getLookAtDir();

        ofVec3f v = mat_pos - cam_pos;
        float dist = v.dot(cam_n);
        ofVec3f mat_plane_pos = mat_pos - (cam_n * dist);

        ofVec3f n = mat_pos - mat_plane_pos;
        n.normalize();

        ofVec3f ray_end = cam.screenToWorld(ofVec3f(x + m_lockedCursor_x, y + m_lockedCursor_y, 1.0f));
        ofVec3f ray_dir = cam_pos - ray_end;
        ray_dir.normalize();

        float t = -((mat_pos - cam_pos).dot(n)) / n.dot(ray_dir);

        m_translation = cam_pos - (ray_dir * t);
      } else {
        ofVec3f df = intersection - m_lockedPoint;
        switch (m_currTranslation)
        {
          case TRANSLATION_X:  df = ofVec3f(df.dot(axe_x),0,0); break;
          case TRANSLATION_Y:  df = ofVec3f(0,df.dot(axe_y),0); break;
          case TRANSLATION_Z:  df = ofVec3f(0,0,df.dot(axe_z)); break;
          case TRANSLATION_XY: df = ofVec3f(df.dot(axe_x),df.dot(axe_y),0); break;
          case TRANSLATION_XZ: df = ofVec3f(df.dot(axe_x),0,df.dot(axe_z)); break;
          case TRANSLATION_YZ: df = ofVec3f(0,df.dot(axe_y),df.dot(axe_z)); break;
        }
        ofVec3f vec = (df.x * axe_x) + (df.y * axe_y) + (df.z * axe_z);

        m_translation = m_translationSaved + vec;
      }
    }
    break;
  }
}

void ofxManipulator::mouseReleased(ofMouseEventArgs &mouse)
{
  int x = mouse.x;
  int y = mouse.y;
  int button = mouse.button;

  if (button != OF_MOUSE_BUTTON_LEFT)
    return;

  m_currScale = SCALE_NONE;
  m_currRotation = ROTATION_NONE;
  m_currTranslation = TRANSLATION_NONE;
}

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

float ofxManipulator::computeScreenFactor()
{
  ofMatrix4x4 viewproj = m_view * m_proj;
  ofVec3f origin = m_translation;
  ofVec4f transform = ofVec4f(origin.x, origin.y, origin.z, 1.0f);
  transform = transform * viewproj;
  return transform.w * 0.2f * m_manipulatorScale;
}

void ofxManipulator::getCurrScale(SCALE_TYPE &type, unsigned int x, unsigned int y)
{
  m_lockedCursor_x = x;
  m_lockedCursor_y = y;
  m_scaleSaved = m_scale;

  ofVec3f ray_origin;
  ofVec3f ray_direction;

  createRay(x, y, ray_origin, ray_direction);

  ofMatrix4x4 m;
  m.rotate(m_rotation);
  m.translate(m_translation);
  ofVec3f axe_x = m.getRowAsVec3f(0);
  ofVec3f axe_y = m.getRowAsVec3f(1);
  ofVec3f axe_z = m.getRowAsVec3f(2);

  ofVec3f df;

  df = raytrace(ray_origin, ray_direction, axe_y, m);

  if( (df.x < 0.1f) && (df.z < 0.1f) && (df.x > -0.1f) && (df.z > -0.1f) ) { type = SCALE_XYZ; return; }
  else
  {
    df = raytrace(ray_origin, ray_direction, axe_z, m);

    if( (df.x < 0.1f) && (df.y < 0.1f) && (df.x > -0.1f) && (df.y > -0.1f) ) { type = SCALE_XYZ; return; }
    else
    {
      df = raytrace(ray_origin, ray_direction, axe_x, m);

      if( (df.y < 0.1f) && (df.z < 0.1f) && (df.y > -0.1f) && (df.z > -0.1f) ) { type = SCALE_XYZ; return; }
    }
  }

  df = raytrace(ray_origin, ray_direction, axe_y, m);

  if      ( (df.x >= 0.f) && (df.x <= 1.f) && (fabs(df.z) < 0.1f) )          { type = SCALE_X;  return; }
  else if ( (df.z >= 0.f) && (df.z <= 1.f) && (fabs(df.x) < 0.1f) )          { type = SCALE_Z;  return; }
  else if ( (df.x < 0.5f) && (df.z < 0.5f) && (df.x > 0.f) && (df.z > 0.f) ) { type = SCALE_XZ; return; }
  else
  {
    df = raytrace(ray_origin, ray_direction, axe_z, m);

    if      ( (df.x >= 0.f) && (df.x <= 1.f) && (fabs(df.y) < 0.1f) )          { type = SCALE_X;  return; }
    else if ( (df.y >= 0.f) && (df.y <= 1.f) && (fabs(df.x) < 0.1f) )          { type = SCALE_Y;  return; }
    else if ( (df.x < 0.5f) && (df.y < 0.5f) && (df.x > 0.f) && (df.y > 0.f) ) { type = SCALE_XY; return; }
    else
    {
      df = raytrace(ray_origin, ray_direction, axe_x, m);

      if      ( (df.y >= 0.f) && (df.y <= 1.f) && (fabs(df.z) < 0.1f) )          { type = SCALE_Y;  return; }
      else if ( (df.z >= 0.f) && (df.z <= 1.f) && (fabs(df.y) < 0.1f) )          { type = SCALE_Z;  return; }
      else if ( (df.y < 0.5f) && (df.z < 0.5f) && (df.y > 0.f) && (df.z > 0.f) ) { type = SCALE_YZ; return; }
    }
  }

  type = SCALE_NONE;
}

void ofxManipulator::getCurrRotation(ROTATION_TYPE &type, unsigned int x, unsigned int y)
{
  ofVec3f ray_origin;
  ofVec3f ray_direction;

  createRay(x, y, ray_origin, ray_direction);

  ofMatrix4x4 m;
  m.rotate(m_rotation);
  ofVec3f axe_x = m.getRowAsVec3f(0);
  ofVec3f axe_y = m.getRowAsVec3f(1);
  ofVec3f axe_z = m.getRowAsVec3f(2);

  if (checkRotationPlane(axe_x, 1.0f, ray_origin, ray_direction)) { type = ROTATION_X; return; }
  if (checkRotationPlane(axe_y, 1.0f, ray_origin, ray_direction)) { type = ROTATION_Y; return; }
  if (checkRotationPlane(axe_z, 1.0f, ray_origin, ray_direction)) { type = ROTATION_Z; return; }

  ofVec3f direction = m_translation - m_viewInverse.getTranslation();
  direction.normalize();

  if (checkRotationPlane(direction, 1.2f, ray_origin, ray_direction)) { type = ROTATION_SCREEN; return; }

  type = ROTATION_NONE;
}

void ofxManipulator::getCurrTranslation(TRANSLATION_TYPE &type, unsigned int x, unsigned int y)
{
  ofCamera cam;
  cam.setTransformMatrix(m_viewInverse);
  ofVec3f mat_screenCoord = cam.worldToScreen(m_translationSaved);
  m_lockedCursor_x = mat_screenCoord.x - x;
  m_lockedCursor_y = mat_screenCoord.y - y;
  m_translationSaved = m_translation;

  ofVec3f ray_origin;
  ofVec3f ray_direction;

  createRay(x, y, ray_origin, ray_direction);

  ofMatrix4x4 m;
  m.rotate(m_rotation);
  m.translate(m_translation);
  ofVec3f axe_x = m.getRowAsVec3f(0);
  ofVec3f axe_y = m.getRowAsVec3f(1);
  ofVec3f axe_z = m.getRowAsVec3f(2);

  ofVec3f df;

  df = raytrace(ray_origin, ray_direction, axe_y, m);

  if( (df.x < 0.1f) && (df.z < 0.1f) && (df.x > -0.1f) && (df.z > -0.1f) ) { type = TRANSLATION_XYZ; return; }
  else
  {
    df = raytrace(ray_origin, ray_direction, axe_z, m);

    if( (df.x < 0.1f) && (df.y < 0.1f) && (df.x > -0.1f) && (df.y > -0.1f) ) { type = TRANSLATION_XYZ; return; }
    else
    {
      df = raytrace(ray_origin, ray_direction, axe_x, m);

      if( (df.y < 0.1f) && (df.z < 0.1f) && (df.y > -0.1f) && (df.z > -0.1f) ) { type = TRANSLATION_XYZ; return; }
    }
  }

  df = raytrace(ray_origin, ray_direction, axe_y, m);

  if      ( (df.x >= 0.f) && (df.x <= 1.f) && (fabs(df.z) < 0.1f) )          { type = TRANSLATION_X;  return; }
  else if ( (df.z >= 0.f) && (df.z <= 1.f) && (fabs(df.x) < 0.1f) )          { type = TRANSLATION_Z;  return; }
  else if ( (df.x < 0.5f) && (df.z < 0.5f) && (df.x > 0.f) && (df.z > 0.f) ) { type = TRANSLATION_XZ; return; }
  else
  {
    df = raytrace(ray_origin, ray_direction, axe_z, m);

    if      ( (df.x >= 0.f) && (df.x <= 1.f) && (fabs(df.y) < 0.1f) )          { type = TRANSLATION_X;  return; }
    else if ( (df.y >= 0.f) && (df.y <= 1.f) && (fabs(df.x) < 0.1f) )          { type = TRANSLATION_Y;  return; }
    else if ( (df.x < 0.5f) && (df.y < 0.5f) && (df.x > 0.f) && (df.y > 0.f) ) { type = TRANSLATION_XY; return; }
    else
    {
      df = raytrace(ray_origin, ray_direction, axe_x, m);

      if      ( (df.y >= 0.f) && (df.y <= 1.f) && (fabs(df.z) < 0.1f) )          { type = TRANSLATION_Y;   return; }
      else if ( (df.z >= 0.f) && (df.z <= 1.f) && (fabs(df.y) < 0.1f) )          { type = TRANSLATION_Z;   return; }
      else if ( (df.y < 0.5f) && (df.z < 0.5f) && (df.y > 0.f) && (df.z > 0.f) ) { type = TRANSLATION_YZ;  return; }
    }
  }

  type = TRANSLATION_NONE;
}

void ofxManipulator::createRay(float x, float y, ofVec3f &ray_origin, ofVec3f &ray_dir)
{
  ofMatrix4x4 view_inverse = m_viewInverse;
  ofMatrix4x4 proj = m_proj;

  float w = ofGetWidth();
  float h = ofGetHeight();

  ofVec3f screen_space(
     (((2.0f * x) / w) - 1.0f) / proj._mat[0][0],
    -(((2.0f * y) / h) - 1.0f) / proj._mat[1][1],
    -1.0f
  );

  auto transform = [] (ofVec3f v, ofMatrix4x4 m)
  {
    ofVec3f vec;
    vec.x = v.x * m._mat[0][0] + v.y * m._mat[1][0] + v.z * m._mat[2][0];
    vec.y = v.x * m._mat[0][1] + v.y * m._mat[1][1] + v.z * m._mat[2][1];
    vec.z = v.x * m._mat[0][2] + v.y * m._mat[1][2] + v.z * m._mat[2][2];
    return vec;
  };

  ray_origin = view_inverse.getTranslation();

  ray_dir = transform(screen_space, view_inverse);
  ray_dir.normalize();
}

void ofxManipulator::intersectRay(ofVec4f &m_plane, ofVec3f &inter_point, ofVec3f position, ofVec3f direction)
{
  float den = m_plane.dot(direction);
  ofVec3f tmp = (m_plane * m_plane.w) - position;
  inter_point = position + (m_plane.dot(tmp) / den) * direction;
}

ofVec3f ofxManipulator::raytrace(ofVec3f ray_origin, ofVec3f ray_dir, ofVec3f normal, ofMatrix4x4 matrix)
{
  ofVec3f df;
  ofVec3f intersection;

  m_plane = createPlane(m_translation, normal);
  intersectRay(m_plane, intersection, ray_origin, ray_dir);
  df = intersection * matrix.getInverse();
  m_lockedPoint = intersection;
  df /= m_screenFactor;

  return df;
}

ofVec4f ofxManipulator::createPlane(ofVec3f p, ofVec3f n)
{
  ofVec4f m_plane;
  ofVec3f normal;

  float length = 1.0f / sqrtf( (n.x * n.x) + (n.y * n.y) + (n.z * n.z) );

  normal = n * length;

  m_plane.x = normal.x;
  m_plane.y = normal.y;
  m_plane.z = normal.z;
  m_plane.w = normal.dot(p);

  return m_plane;
}

bool ofxManipulator::checkRotationPlane(ofVec3f normal, float factor, ofVec3f ray_origin, ofVec3f ray_direction)
{
  ofVec3f df;
  ofVec3f intersection;
  ofVec3f translation = m_translation;

  m_plane = createPlane(translation, normal);
  intersectRay(m_plane, intersection, ray_origin, ray_direction);
  df = intersection - translation;
  df /= m_screenFactor;

  float dflen = (df.length() / factor);
  if ((dflen > 0.9f) && (dflen < 1.1f))
  {
    m_rotationSaved = m_rotation;
    m_lockedPoint = df;
    m_lockedPoint.normalize();
    m_rotate_x = m_lockedPoint;
    m_rotate_y = m_lockedPoint;
    m_rotate_y.cross(normal);
    m_rotate_y.normalize();
    m_rotate_x *= factor;
    m_rotate_y *= factor;
    m_angleRad = 0.0f;
    m_axis = normal;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// DRAW FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void ofxManipulator::drawCircle(ofVec3f origin, ofVec3f vtx, ofVec3f vty, ofColor color)
{
  const int size = 50;
  vector<ofVec3f> vertices;
  vertices.resize(size);
  for (int i = 0; i < size; i++) {
    ofVec3f vt;
    vt  = vtx * cos((2 * PI / size) * i);
    vt += vty * sin((2 * PI / size) * i);
    vt += origin;
    vertices[i] = vt;
  }

  ofSetColor(color);
  ofMesh(OF_PRIMITIVE_LINE_LOOP, vertices).draw();
}

void ofxManipulator::drawAxis(ofVec3f origin, ofVec3f m_axis, ofVec3f vtx, ofVec3f vty, float fct, float fct2, ofColor color)
{
  vector<ofVec3f> cone_mesh;
  cone_mesh.resize(93);
  for (int i = 0, j = 0; i <= 30; i++) {
    ofVec3f pt;
    pt  = vtx * cos(((2 * PI) / 30.0f) * i) * fct;
    pt += vty * sin(((2 * PI) / 30.0f) * i) * fct;
    pt += m_axis * fct2;
    pt += origin;
    cone_mesh[j++] = pt;
    pt  = vtx * cos(((2 * PI) / 30.0f) * (i + 1)) * fct;
    pt += vty * sin(((2 * PI) / 30.0f) * (i + 1)) * fct;
    pt += m_axis * fct2;
    pt += origin;
    cone_mesh[j++] = pt;
    cone_mesh[j++] = origin + m_axis;
  }

  ofSetColor(color);
  ofMesh(OF_PRIMITIVE_LINES, {origin, origin + m_axis}).draw();
  ofMesh(OF_PRIMITIVE_TRIANGLE_FAN, cone_mesh).draw();
}

void ofxManipulator::drawScaleAxis(ofVec3f origin, ofVec3f m_axis, ofVec3f vtx, ofColor color)
{
  ofSetColor(color);
  ofMesh(OF_PRIMITIVE_LINES, {origin, origin + m_axis}).draw();

  float box_size = 0.1f * m_screenFactor;

  ofVec3f box_pos = origin;
  box_pos += m_axis.getNormalized() * (m_axis.length() - (box_size * 0.5f));

  ofNode box_node;
  box_node.lookAt(m_axis, vtx);
  box_node.setPosition(box_pos);
  ofPushMatrix();
  ofMultMatrix(box_node.getGlobalTransformMatrix());
  ofDrawBox(box_size);
  ofPopMatrix();
}

void ofxManipulator::drawCamembert(ofVec3f origin, ofVec3f vtx, ofVec3f vty, float ng, ofColor color)
{
  vector<ofVec3f> vertices;
  vertices.resize(52);
  int j = 0;
  vertices[j++] = origin;
  for (int i = 0; i <= 50; i++) {
    ofVec3f vt;
    vt  = vtx * cos((ng / 50) * i);
    vt += vty * sin((ng / 50) * i);
    vt += origin;
    vertices[j++] = vt;
  }

  ofSetColor(color, 255.0f * 0.5f);
  ofMesh(OF_PRIMITIVE_TRIANGLE_FAN, vertices).draw();
  ofMesh(OF_PRIMITIVE_LINE_LOOP, vertices).draw();
}

void ofxManipulator::drawQuad(ofVec3f origin, float size, bool is_selected, ofVec3f m_axis_u, ofVec3f m_axis_v, ofColor color)
{
  origin += ((m_axis_u + m_axis_v) * size) * 0.2f;
  m_axis_u *= 0.8f;
  m_axis_v *= 0.8f;

  vector<ofVec3f> pts;
  pts.resize(4);
  pts[0] = origin;
  pts[1] = origin + (m_axis_u * size);
  pts[2] = origin + (m_axis_u + m_axis_v) * size;
  pts[3] = origin + (m_axis_v * size);

  ofSetColor(color, 255.0f * (is_selected? 0.8f : 0.4f));
  ofMesh(OF_PRIMITIVE_TRIANGLE_FAN, pts).draw();
  ofSetColor(color, 255.0f * (is_selected? 1.0f : 0.8f));
  ofMesh(OF_PRIMITIVE_LINE_LOOP, pts).draw();
}

void ofxManipulator::drawTriangle(ofVec3f origin, float size, bool is_selected, ofVec3f m_axis_u, ofVec3f m_axis_v, ofColor color)
{
  origin += ((m_axis_u + m_axis_v) * size) * 0.2f;
  m_axis_u *= 0.8f;
  m_axis_v *= 0.8f;

  vector<ofVec3f> pts;
  pts.resize(3);
  pts[0] = origin;
  pts[1] = (m_axis_u * size) + origin;
  pts[2] = (m_axis_v * size) + origin;

  ofSetColor(color, 255.0f * (is_selected? 0.8f : 0.4f));
  ofMesh(OF_PRIMITIVE_TRIANGLES, pts).draw();
  ofSetColor(color, 255.0f * (is_selected? 1.0f : 0.8f));
  ofMesh(OF_PRIMITIVE_LINE_LOOP, pts).draw();
}
