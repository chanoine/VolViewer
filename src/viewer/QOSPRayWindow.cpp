// ======================================================================== //
// Copyright 2009-2014 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //


#include "QOSPRayWindow.h"

#include <iostream>
#include <fstream>

#include <sys/resource.h>

QOSPRayWindow::QOSPRayWindow(QMainWindow *parent, 
                             OSPRenderer renderer, 
                             bool showFrameRate) 
  : parent(parent), 
    showFrameRate(showFrameRate), 
    frameCount(0), 
    renderingEnabled(false), 
    rotationRate(0.f), 
    benchmarkWarmUpFrames(0), 
    benchmarkFrames(0), 
    frameBuffer(NULL)
{
  this->renderer = renderer;

	setFocusPolicy(Qt::StrongFocus);

	camera.setRenderer(renderer);
	camera.setPos(osp::vec3f(255.5, 255, 255.5));
	camera.setDir(osp::vec3f(0.0, -1700.5, 0.0));
	camera.commit();
}

QOSPRayWindow::~QOSPRayWindow()
{
  if(frameBuffer)
		ospFreeFrameBuffer(frameBuffer);
}

void
QOSPRayWindow::saveState(std::ostream& out)
{
	camera.saveState(out);
}

void
QOSPRayWindow::loadState(std::istream& in)
{
	camera.loadState(in);
}

void QOSPRayWindow::setRenderingEnabled(bool renderingEnabled)
{
  this->renderingEnabled = renderingEnabled;

  // trigger render if true
  if(renderingEnabled == true)
    {
      updateGL();
    }
}

void QOSPRayWindow::setRotationRate(float rotationRate)
{
  this->rotationRate = rotationRate;
}

void QOSPRayWindow::setBenchmarkParameters(int benchmarkWarmUpFrames, int benchmarkFrames)
{
  this->benchmarkWarmUpFrames = benchmarkWarmUpFrames;
  this->benchmarkFrames = benchmarkFrames;
}

void QOSPRayWindow::paintGL()
{
  if(!renderingEnabled || !frameBuffer || !renderer)
    {
      return;
    }

  // if we're benchmarking and we've completed the required number of warm-up frames, start the timer
  if(benchmarkFrames > 0 && frameCount == benchmarkWarmUpFrames)
    {
      std::cout << "starting benchmark timer" << std::endl;
      benchmarkTimer.start();
    }

	camera.commit();

  renderFrameTimer.start();

  ospRenderFrame(frameBuffer, renderer);

  double framesPerSecond = 1000.0 / renderFrameTimer.elapsed();
  char title[1024];  sprintf(title, "OSPRay Volume Viewer (%.4f fps)", framesPerSecond);
  if (showFrameRate == true) parent->setWindowTitle(title);

  uint32 * mappedFrameBuffer = (unsigned int *)ospMapFrameBuffer(frameBuffer);

  glDrawPixels(windowSize.x, windowSize.y, GL_RGBA, GL_UNSIGNED_BYTE, mappedFrameBuffer);

  ospUnmapFrameBuffer(mappedFrameBuffer, frameBuffer);

  // automatic rotation
  if(rotationRate != 0.f)
    {
      camera.rotateCenter(rotationRate, 0.f);
    }

  // increment frame counter
  frameCount++;

  // quit if we're benchmarking and have exceeded the needed number of frames
  if(benchmarkFrames > 0 && frameCount >= benchmarkWarmUpFrames + benchmarkFrames)
    {
      float elapsedSeconds = float(benchmarkTimer.elapsed()) / 1000.f;

      std::cout << "benchmark: " << elapsedSeconds << " elapsed seconds ==> " << float(benchmarkFrames) / elapsedSeconds << " fps" << std::endl;

      QCoreApplication::quit();
    }

  // force continuous rendering if we have automatic rotation or benchmarking enabled
  if(rotationRate != 0.f || benchmarkFrames > 0)
    {
      update();
    }
}

void QOSPRayWindow::resizeGL(int width, int height)
{
  windowSize = osp::vec2i(width, height);

  // reallocate OSPRay framebuffer for new size
  if(frameBuffer)
    {
      ospFreeFrameBuffer(frameBuffer);
    }

  frameBuffer = ospNewFrameBuffer(windowSize, OSP_RGBA_I8);

  // update camera aspect ratio
  camera.setAspect(float(width) / float(height));
	camera.commit();

  // update OpenGL camera and force redraw
  glViewport(0, 0, width, height);
  updateGL();
}

void QOSPRayWindow::mousePressEvent(QMouseEvent * event)
{
  lastMousePosition = event->pos();
}

void QOSPRayWindow::mouseReleaseEvent(QMouseEvent * event)
{
  lastMousePosition = event->pos();
}

void QOSPRayWindow::mouseMoveEvent(QMouseEvent * event)
{
  int dx = event->x() - lastMousePosition.x();
  int dy = event->y() - lastMousePosition.y();

  if(event->buttons() & Qt::LeftButton)
    {
      // camera rotation about center point
      const float rotationSpeed = 0.003f;

      float du = dx * rotationSpeed;
      float dv = dy * rotationSpeed;

      camera.rotateCenter(du, dv);
    }
  else if(event->buttons() & Qt::RightButton)
    {
			camera.zoom(dy);
    }

  lastMousePosition = event->pos();
  updateGL();
}

void QOSPRayWindow::keyPressEvent(QKeyEvent *event)
{
	if (event->text().toStdString()[0] == '+')
	{
			camera.zoom(5);
			updateGL();
	}
	else if (event->text().toStdString()[0] == '-')
	{
			camera.zoom(-5);
			updateGL();
	}
}