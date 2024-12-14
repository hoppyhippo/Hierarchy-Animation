
//
//  Starter file for Project 3 - Skeleton Builder
//
//  This file includes functionality that supports selection and translate/rotation
//  of scene objects using the mouse.
//
//  Modifer keys for rotatation are x, y and z keys (for each axis of rotation)
//
//  (c) Kevin M. Smith  - 24 September 2018
//

#include "ofMain.h"
#include "box.h"
#include "Primitives.h"
#include "ofxGui.h"

class KeyFrame {
public:
	int frame = -1;     //  -1 => no key is set;
	glm::vec3 position = glm::vec3(0, 0, 0);   // translate channel
	glm::vec3 rotation = glm::vec3(0, 0, 0);   // rotate channel
	glm::vec3 scale = glm::vec3(1, 1, 1);   // rotate channel
	SceneObject* obj = NULL;                   // object that is keyframed
};

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	static void drawAxis(glm::mat4 transform = glm::mat4(1.0), float len = 1.0);
	bool mouseToDragPlane(int x, int y, glm::vec3& point);
	void printChannels(SceneObject*);
	bool objSelected() { return (selected.size() ? true : false); };
	void addJoint();
	void deleteObject();
	//void saveToFile(string& filename);
	void saveToFile();
	void loadFile();
	void clearSelectionList() {
		for (int i = 0; i < selected.size(); i++) {
			selected[i]->isSelected = false;
		}
		selected.clear();
	}

	// key framing
//
	void nextFrame() {
		if (bInPlayback) {
			frame += 1;
		}
		else {
			frame += 5;
		}
		if (frame > frameEnd) {
			frame = frameBegin;
		}
	}
	void prevFrame() {
		frame = (frame == frameBegin ? frame : frame - 5);
		if (frame < frameBegin) {
			frame = frameBegin;
		}
	}
	void startPlayback() {
		bInPlayback = true;
	}

	void stopPlayback() {
		bInPlayback = false;
	}

	// check if both keys set
	//
	bool keyFramesSet() {
		bool set = false;
		for (auto kf : keyFrames) {
			if (kf.frame != -1) {
				set = true;
			}
		}
		return (set);
	}

	// linear interpolation between two keyframes
	//
	glm::vec3 linearInterp(int frame, int frameStart, int frameEnd, const glm::vec3& start, const glm::vec3& end) {
		return mapVec(frame, frameStart, frameEnd, start, end);
	}

	// example non-linear interpolation between two keyframes (ease-in ease-out)
	//
	glm::vec3 easeInterp(int frame, int frameStart, int frameEnd, const glm::vec3& start, const glm::vec3& end) {

		// normalize range (0 to 1) and input to ease formula
		//
		float s = ease(ofMap(frame, frameStart, frameEnd, 0.0, 1.0));

		return mapVec(s, 0.0, 1.0, start, end);
	}


	//  ease-in and ease-out interpolation between two key frames
	//  this function produces a sigmoid curve normalized in x, y in (0 to 1);
	//
	float ease(float x) {
		return (x * x / (x * x + (1 - x) * (1 - x)));
	}

	// helper functions to use ofMap on a vector
	//
	// input a float value in a float range, output a vector 
	//
	glm::vec3 mapVec(float val, float start, float end, const glm::vec3& outStart, const glm::vec3& outEnd) {
		return glm::vec3(
			ofMap(val, start, end, outStart.x, outEnd.x),
			ofMap(val, start, end, outStart.y, outEnd.y),
			ofMap(val, start, end, outStart.z, outEnd.z));
	}

	// input a vec3 value in a vec3 range, output a vector 
	//
	glm::vec3 mapVec(const glm::vec3& val, const glm::vec3& start, const glm::vec3& end, const glm::vec3& outStart, const glm::vec3& outEnd) {
		return glm::vec3(
			ofMap(val.x, start.x, end.x, outStart.x, outEnd.x),
			ofMap(val.y, start.y, end.y, outStart.y, outEnd.y),
			ofMap(val.z, start.z, end.z, outStart.z, outEnd.z));
	}

	// set keyframe for SceneObject at current frame
	// call this function the first time and key1 is set
	// call this function again and key2 is set.
	// this "cycles" until you call resetKeyFrames();
	//
	void setKeyFrame() {
		if (selected.empty()) {
			cout << "No object selected. Cannot set keyframe." << endl;
			return;
		}

		for (auto obj : selected) {
			Joint* joint = dynamic_cast<Joint*>(obj);
			if (joint) {
				KeyFrame keyFrame;
				keyFrame.frame = frame;
				keyFrame.position = joint->position;
				keyFrame.rotation = joint->rotation;
				keyFrame.scale = joint->scale;

				joint->keyFrames.push_back(keyFrame);
				cout << "Setting keyframe at frame: " << frame << endl;
			}
			else {
				cout << "Cannot set keyframe." << endl;
			}
		}
	}

	void interpolateKeyFrames() {
		for (auto& obj : scene) {
			Joint* joint = dynamic_cast<Joint*>(obj);
			if (joint && joint->keyFrames.size() > 1) {
				for (size_t i = 0; i < joint->keyFrames.size() - 1; i++) {
					KeyFrame& kf1 = joint->keyFrames[i];
					KeyFrame& kf2 = joint->keyFrames[i + 1];

					if (frame >= kf1.frame && frame <= kf2.frame) {
						if (useEaseInterpolation) {
							// Use ease interpolation
							joint->position = easeInterp(frame, kf1.frame, kf2.frame, kf1.position, kf2.position);
							joint->rotation = easeInterp(frame, kf1.frame, kf2.frame, kf1.rotation, kf2.rotation);
							joint->scale = easeInterp(frame, kf1.frame, kf2.frame, kf1.scale, kf2.scale);
						}
						else {
							// Use linear interpolation
							joint->position = linearInterp(frame, kf1.frame, kf2.frame, kf1.position, kf2.position);
							joint->rotation = linearInterp(frame, kf1.frame, kf2.frame, kf1.rotation, kf2.rotation);
							joint->scale = linearInterp(frame, kf1.frame, kf2.frame, kf1.scale, kf2.scale);
						}
						break;
					}
				}
			}
		}
	}

	void deleteKeyFrame() {
		if (!objSelected()) {
			cout << "No object selected. Cannot delete keyframe." << endl;
			return;
		}

		for (auto obj : selected) {
			Joint* joint = dynamic_cast<Joint*>(obj);
			if (joint) {
				auto it = std::remove_if(joint->keyFrames.begin(), joint->keyFrames.end(),
					[&](const KeyFrame& k) { return k.frame == frame; });

				if (it != joint->keyFrames.end()) {
					joint->keyFrames.erase(it, joint->keyFrames.end());
					cout << "Deleted keyframe for frame: " << frame << endl;
				}
				else {
					cout << "No keyframe found at frame: " << frame << endl;
				}
			}
			else {
				cout << "Selected object is not a joint. Cannot delete keyframe." << endl;
			}
		}
	}

	// reset key frames
	//
	void resetKeyFrames() {
		/*for (auto kf : keyFrames) {
			kf.frame = -1;
			kf.obj = NULL;
		}
		bKey2Next = false;*/

		if (objSelected()) {
			Joint* selectedJoint = dynamic_cast<Joint*>(selected[0]);
			if (selectedJoint) {
				selectedJoint->keyFrames.clear();
				bKey2Next = false;
			}
		}
	}

	void resetRotation() {
		if (objSelected()) {
			for (auto& obj : selected) {
				Joint* joint = dynamic_cast<Joint*>(obj);
				if (joint) {
					joint->rotation = glm::vec3(0, 0, 0);
					cout << "Rotations reset to zero, " << joint->name << endl;
				}
			}
		}
		else {
			cout << "No joint selected to reset rotation." << endl;
		}
	}

	// Lights
	//
	ofLight light1;

	// Cameras
	//
	ofEasyCam  mainCam;
	ofCamera sideCam;
	ofCamera topCam;
	ofCamera* theCam;    // set to current camera either mainCam or sideCam

	// Materials
	//
	ofMaterial material;


	// scene components
	//
	ofxToggle useEaseInterpolation;
	vector<SceneObject*> scene;
	vector<SceneObject*> selected;
	ofPlanePrimitive plane;
	int jointCounter = 0;
	ofxPanel gui;
	ofxLabel rotationText;

	vector<KeyFrame> keyFrames;
	int frame = 1;         // current frame
	int frameBegin = 1;     // first frame of playback range;
	int frameEnd = 501;     // last frame of playback range;
	bool bInPlayback = false;  // true => we are in playback mode
	bool bKey2Next = false;

	// state
	bool bDrag = false;
	bool bHide = true;
	bool bAltKeyDown = false;
	bool bRotateX = false;
	bool bRotateY = false;
	bool bRotateZ = false;
	bool bAnimate = false;
	bool bCtrlKeyDown = false;
	glm::vec3 lastPoint;

	ofxPanel keyframePanel;
	ofxButton addKeyframeBtn;
	ofxButton deleteKeyframeBtn;
	ofxButton resetKeyframesBtn;
	ofxButton resetRotationBtn;
	ofxIntSlider frameSlider;
	ofxButton saveBtn;
	ofxButton loadBtn;

	// Timeline constants
	const int timelineY = 700;
	const int timelineHeight = 30;
	const int timelineWidth = 1500;
	const int keyframeMarkerSize = 8;

	void setupKeyframeUI();
	void drawTimeline();
	bool isMouseOverTimeline(int x, int y);
	void handleTimelineClick(int x, int y);
	void frameChanged(int& f);
};