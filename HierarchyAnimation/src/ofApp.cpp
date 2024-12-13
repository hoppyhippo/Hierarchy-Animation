
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


#include "ofApp.h"
#include <fstream>
#include <sstream>


//--------------------------------------------------------------
//
void ofApp::setup() {

	ofSetBackgroundColor(ofColor::black);
	ofEnableDepthTest();
	mainCam.setDistance(15);
	mainCam.setNearClip(.1);

	sideCam.setPosition(40, 0, 0);
	sideCam.lookAt(glm::vec3(0, 0, 0));
	topCam.setNearClip(.1);
	topCam.setPosition(0, 16, 0);
	topCam.lookAt(glm::vec3(0, 0, 0));
	ofSetSmoothLighting(true);

	// setup one point light
	//
	light1.enable();
	light1.setPosition(5, 5, 0);
	light1.setDiffuseColor(ofColor(255.f, 255.f, 255.f));
	light1.setSpecularColor(ofColor(255.f, 255.f, 255.f));

	theCam = &mainCam;

	//  create a scene consisting of a ground plane with 2x2 blocks
	//  arranged in semi-random positions, scales and rotations
	//
	// ground plane
	//
	scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0)));

	/*Sphere* sun = new Sphere(glm::vec3(0,0,0), 3.0, ofColor::yellow);
	Sphere* earth = new Sphere(glm::vec3(7,1,0), 1.0, ofColor::blue);
	Sphere* moon = new Sphere(glm::vec3(2,0,0), 0.25);
	sun->addChild(earth);
	earth->addChild(moon);

	scene.push_back(sun);
	scene.push_back(earth);
	scene.push_back(moon);*/

	ofxGuiSetDefaultWidth(500);
	gui.setup();
	gui.setPosition(5, 25);
	gui.add(rotationText.setup("Rotation", "x: 0.0, y: 0.0, z: 0.0"));

	setupKeyframeUI();
}


//--------------------------------------------------------------
void ofApp::update() {
	//scene[1]->rotation.y++;
	//scene[2]->rotation.y++;
	//scene[2]->scale += .1;
	if (objSelected()) {
		glm::vec3 rotation = selected[0]->rotation;
		rotationText =
			"X: " + std::to_string(rotation.x) +
			", Y: " + std::to_string(rotation.y) +
			", Z: " + std::to_string(rotation.z);
	}
	else {
		rotationText = "No object selected";
	}

	//if (bAnimate) {

	//}
	if (bInPlayback) {
		nextFrame();
		interpolateKeyFrames();
	}

	// if keyframes are set and the current frame is between
	// the two keys, then we need to calculate "in-between" values
	// for position/rotation of the object that is keyframed.
	// we then "set" the keyframed objects' position/rotation to 
	// the calculated value.
	//
	//if (keyFramesSet() && (frame >= key1.frame && frame <= key2.frame)) {
	//	key1.obj->position = linearInterp(frame, key1.frame, key2.frame, key1.position, key2.position);

	//	// add more interpoliation types/channels here...
	//	key1.obj->rotation = linearInterp(frame, key1.frame, key2.frame, key1.rotation, key2.rotation);
	//	key1.obj->scale = linearInterp(frame, key1.frame, key2.frame, key1.scale, key2.scale);

	//	// ease interpolation
	//	key1.obj->position = easeInterp(frame, key1.frame, key2.frame, key1.position, key2.position);
	//	key1.obj->rotation = easeInterp(frame, key1.frame, key2.frame, key1.rotation, key2.rotation);
	//	key1.obj->scale = easeInterp(frame, key1.frame, key2.frame, key1.scale, key2.scale);
	//}
	//if (keyFramesSet()) {
	//	interpolateKeyFrames();
	//}
}

//--------------------------------------------------------------
void ofApp::draw() {

	theCam->begin();
	ofNoFill();
	drawAxis();
	ofEnableLighting();

	//  draw the objects in scene
	//
	material.begin();
	ofFill();
	for (int i = 0; i < scene.size(); i++) {
		if (std::find(selected.begin(), selected.end(), scene[i]) != selected.end()) {
			ofSetColor(ofColor::white);
		}
		else {
			ofSetColor(scene[i]->diffuseColor);
		}
		scene[i]->draw();
	}

	material.end();
	ofDisableLighting();
	ofDisableDepthTest();
	theCam->end();
	gui.draw();

	keyframePanel.draw();
	drawTimeline();

	// Display the current frame and total frames
	std::string str1;
	str1 += "Frame: " + std::to_string(frame) + " of " + std::to_string(frameEnd - frameBegin + 1);
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str1, 5, 15);

	std::ostringstream buf;
	ofSetColor(ofColor::lightGreen);

	int xOffset = ofGetWidth() - 150;
	int yOffset = 25;

	for (auto& obj : selected) {
		Joint* joint = dynamic_cast<Joint*>(obj);
		if (joint) {
			ofSetColor(ofColor::yellow);
			ofDrawBitmapString("Joint: " + joint->name, xOffset, yOffset);
			yOffset += 20;

			ofSetColor(ofColor::lightGreen);
			for (int i = 0; i < joint->keyFrames.size(); i++) {
				const KeyFrame& kf = joint->keyFrames[i];
				std::ostringstream buf;
				buf << "Keyframe " << (i + 1) << ": ";
				buf << "Frame: " << kf.frame << ", " << endl;
				buf << "Position: (" << kf.position.x << ", " << kf.position.y << ", " << kf.position.z << "), ";
				buf << "Rotation: (" << kf.rotation.x << ", " << kf.rotation.y << ", " << kf.rotation.z << "), ";
				buf << "Scale: (" << kf.scale.x << ", " << kf.scale.y << ", " << kf.scale.z << ")";
				ofDrawBitmapString(buf.str(), 5, yOffset + 30);
				yOffset += 30;
			}
		}
	}
	ofSetColor(ofColor::white);

}

void ofApp::setupKeyframeUI() {
	keyframePanel.setup("Keyframe Controls", "keyframe_settings.xml", 520, 10);

	addKeyframeBtn.setup("Add Keyframe, k");
	deleteKeyframeBtn.setup("Delete Keyframe, del");
	resetKeyframesBtn.setup("Reset Keyframes, d");
	resetRotationBtn.setup("Reset Rotation, r");
	saveBtn.setup("Save, s");
	loadBtn.setup("Load, l");
	frameSlider.setup("Frame", frame, frameBegin, frameEnd);
	useEaseInterpolation.setup("Use Ease Interpolation", false);

	keyframePanel.add(&addKeyframeBtn);
	keyframePanel.add(&deleteKeyframeBtn);
	keyframePanel.add(&resetKeyframesBtn);
	keyframePanel.add(&resetRotationBtn);
	keyframePanel.add(&saveBtn);
	keyframePanel.add(&loadBtn);
	keyframePanel.add(&frameSlider);
	keyframePanel.add(&useEaseInterpolation);

	// Setup event listeners
	addKeyframeBtn.addListener(this, &ofApp::setKeyFrame);
	deleteKeyframeBtn.addListener(this, &ofApp::deleteKeyFrame);
	resetKeyframesBtn.addListener(this, &ofApp::resetKeyFrames);
	resetRotationBtn.addListener(this, &ofApp::resetRotation);
	saveBtn.addListener(this, &ofApp::saveToFile);
	loadBtn.addListener(this, &ofApp::loadFile);
	frameSlider.addListener(this, &ofApp::frameChanged);
}



void ofApp::drawTimeline() {
	// Draw timeline background
	ofSetColor(ofColor::darkGrey);
	ofDrawRectangle(10, timelineY, timelineWidth, timelineHeight);

	// Draw frame markers
	ofSetColor(ofColor::white);
	for (int f = frameBegin; f <= frameEnd; f += 10) {
		float x = ofMap(f, frameBegin, frameEnd, 10, timelineWidth + 10);
		ofDrawLine(x, timelineY, x, timelineY + 10);
		ofDrawBitmapString(ofToString(f), x - 5, timelineY + 25);
	}

	// Draw current frame indicator
	float currentX = ofMap(frame, frameBegin, frameEnd, 10, timelineWidth + 10);
	ofSetColor(ofColor::red);
	ofDrawTriangle(currentX - 10, timelineY - 5,
		currentX + 10, timelineY - 5,
		currentX, timelineY + 5);

	// Draw keyframes for selected joint
	if (objSelected()) {
		Joint* selectedJoint = dynamic_cast<Joint*>(selected[0]);
		if (selectedJoint) {
			int mouseX = ofGetMouseX();
			int mouseY = ofGetMouseY();

			for (const auto& kf : selectedJoint->keyFrames) {
				float x = ofMap(kf.frame, frameBegin, frameEnd, 10, timelineWidth + 10);

				// Check if mouse is hovering over keyframe
				float dist = glm::distance(glm::vec2(mouseX, mouseY),
					glm::vec2(x, timelineY + timelineHeight / 2));

				if (dist < keyframeMarkerSize) {
					// Draw hover effect
					ofSetColor(ofColor::red);
					ofDrawCircle(x, timelineY + timelineHeight / 2, keyframeMarkerSize + 2);

					// Draw tooltip with frame number
					ofSetColor(ofColor::white);
					string info = "Frame: " + ofToString(kf.frame);
					ofDrawBitmapString(info, x - 30, timelineY - 10);
				}

				// Draw normal keyframe marker
				ofSetColor(ofColor::yellow);
				ofDrawCircle(x, timelineY + timelineHeight / 2, keyframeMarkerSize);
			}
		}
	}
}

bool ofApp::isMouseOverTimeline(int x, int y) {
	return (x >= 10 && x <= timelineWidth + 10 &&
		y >= timelineY && y <= timelineY + timelineHeight);
}

void ofApp::handleTimelineClick(int x, int y) {
	if (isMouseOverTimeline(x, y)) {
		int clickedFrame = ofMap(x, 10, timelineWidth + 10, frameBegin, frameEnd);
		frame = ofClamp(clickedFrame, frameBegin, frameEnd);
		frameSlider = frame;
	}
}

void ofApp::frameChanged(int& f) {
	frame = f;
	interpolateKeyFrames();
}

// 
// Draw an XYZ axis in RGB at transform
//
void ofApp::drawAxis(glm::mat4 m, float len) {

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(glm::vec3(m * glm::vec4(0, 0, 0, 1)), glm::vec3(m * glm::vec4(len, 0, 0, 1)));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(glm::vec3(m * glm::vec4(0, 0, 0, 1)), glm::vec3(m * glm::vec4(0, len, 0, 1)));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(glm::vec3(m * glm::vec4(0, 0, 0, 1)), glm::vec3(m * glm::vec4(0, 0, len, 1)));
}

// print C++ code for obj tranformation channels. (for debugging);
//
void ofApp::printChannels(SceneObject* obj) {
	cout << "position = glm::vec3(" << obj->position.x << "," << obj->position.y << "," << obj->position.z << ");" << endl;
	cout << "rotation = glm::vec3(" << obj->rotation.x << "," << obj->rotation.y << "," << obj->rotation.z << ");" << endl;
	cout << "scale = glm::vec3(" << obj->scale.x << "," << obj->scale.y << "," << obj->scale.z << ");" << endl;
}

void ofApp::addJoint() {
	std::string jointName = "joint" + std::to_string(jointCounter);
	jointCounter++;
	Joint* newJoint = new Joint(jointName, 1.0f);

	// set parent
	if (objSelected()) {
		SceneObject* selectedObj = selected[0];
		Joint* parentJoint = dynamic_cast<Joint*>(selectedObj);
		if (parentJoint) {
			parentJoint->addChild(newJoint);
		}
	}

	scene.push_back(newJoint);
	selected.clear();
	selected.push_back(newJoint);
}

void ofApp::deleteObject() {
	if (objSelected()) {
		SceneObject* selectedObj = selected[0];
		// add children to the selected joint's parent
		for (auto child : selectedObj->childList) {
			if (selectedObj->parent) {
				selectedObj->parent->addChild(child);
			}
			else {
				child->parent = nullptr;
			}
		}

		// remove from parent's child list
		if (selectedObj->parent) {
			auto& siblings = selectedObj->parent->childList;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), selectedObj), siblings.end());
		}

		// remove from scene
		scene.erase(std::remove(scene.begin(), scene.end(), selectedObj), scene.end());

		// delete the joint
		delete selectedObj;
		selected.clear();
	}
}

void ofApp::saveToFile() {
	ofFileDialogResult result = ofSystemSaveDialog("animation.txt", "Save");

	if (result.bSuccess) {
		std::string filePath = result.getPath();

		std::ofstream file(filePath);

		if (!file.is_open()) {
			cerr << "File could not be opened for saving: " << filePath << endl;
			return;
		}

		for (auto& obj : scene) {
			// check if the object is a joint
			Joint* joint = dynamic_cast<Joint*>(obj);
			if (joint) {
				file << "Joint: " << joint->name << endl;
				file << "Parent: " << (joint->parent ? joint->parent->name : "None") << endl; // Save parent name
				file << "Position: (" << joint->position.x << ", " << joint->position.y << ", " << joint->position.z << ")" << endl;
				file << "Rotation: (" << joint->rotation.x << ", " << joint->rotation.y << ", " << joint->rotation.z << ")" << endl;
				file << "Scale: (" << joint->scale.x << ", " << joint->scale.y << ", " << joint->scale.z << ")" << endl;

				// write keyframe data
				if (!joint->keyFrames.empty()) {
					file << "KeyFrames:" << endl;
					for (const auto& kf : joint->keyFrames) {
						file << "  Frame: " << kf.frame << endl;
						file << "    Position: (" << kf.position.x << ", " << kf.position.y << ", " << kf.position.z << ")" << endl;
						file << "    Rotation: (" << kf.rotation.x << ", " << kf.rotation.y << ", " << kf.rotation.z << ")" << endl;
						file << "    Scale: (" << kf.scale.x << ", " << kf.scale.y << ", " << kf.scale.z << ")" << endl;
					}
				}
				file << endl;
			}
		}

		file.close();
		cout << "Scene saved to file: " << filePath << endl;
	}
	else {
		cout << "Save canceled." << endl;
	}
}

void ofApp::loadFile() {
	ofFileDialogResult result = ofSystemLoadDialog("Load");

	if (result.bSuccess) {
		string filePath = result.getPath();
		std::ifstream file(filePath);

		if (!file.is_open()) {
			cerr << "Failed to open file: " << filePath << endl;
			return;
		}

		scene.clear();
		std::unordered_map<string, Joint*> joints;
		Joint* currentJoint = nullptr;

		string line;
		while (std::getline(file, line)) {
			std::istringstream stream(line);
			string word;
			stream >> word;

			if (word == "Joint:") {
				string jointName;
				stream >> jointName;

				currentJoint = new Joint(jointName, 1.0f);
				scene.push_back(currentJoint);
				joints[jointName] = currentJoint;
			}
			else if (word == "Parent:" && currentJoint) {
				string parentName;
				stream >> parentName;

				if (parentName != "None" && joints.find(parentName) != joints.end()) {
					currentJoint->parent = joints[parentName];
					joints[parentName]->addChild(currentJoint);
				}
			}
			else if (word == "Position:" && currentJoint) {
				char dummy;
				glm::vec3 position;
				stream >> dummy >> position.x >> dummy >> position.y >> dummy >> position.z >> dummy;
				currentJoint->position = position;
			}
			else if (word == "Rotation:" && currentJoint) {
				char dummy;
				glm::vec3 rotation;
				stream >> dummy >> rotation.x >> dummy >> rotation.y >> dummy >> rotation.z >> dummy;
				currentJoint->rotation = rotation;
			}
			else if (word == "Scale:" && currentJoint) {
				char dummy;
				glm::vec3 scale;
				stream >> dummy >> scale.x >> dummy >> scale.y >> dummy >> scale.z >> dummy;
				currentJoint->scale = scale;
			}
			else if (word == "Frame:" && currentJoint) {
				KeyFrame keyFrame;
				stream >> keyFrame.frame;

				while (std::getline(file, line) && line.find("Frame:") == std::string::npos && !line.empty()) {
					std::istringstream subStream(line);
					string subWord;
					subStream >> subWord;

					if (subWord == "Position:") {
						char dummy;
						glm::vec3 position;
						subStream >> dummy >> position.x >> dummy >> position.y >> dummy >> position.z >> dummy;
						keyFrame.position = position;
					}
					else if (subWord == "Rotation:") {
						char dummy;
						glm::vec3 rotation;
						subStream >> dummy >> rotation.x >> dummy >> rotation.y >> dummy >> rotation.z >> dummy;
						keyFrame.rotation = rotation;
					}
					else if (subWord == "Scale:") {
						char dummy;
						glm::vec3 scale;
						subStream >> dummy >> scale.x >> dummy >> scale.y >> dummy >> scale.z >> dummy;
						keyFrame.scale = scale;
					}
				}

				currentJoint->keyFrames.push_back(keyFrame);

				if (line.find("Frame:") != std::string::npos) {
					std::istringstream frameStream(line);
					frameStream >> word;
					frameStream >> keyFrame.frame;
				}
			}
		}

		for (auto& obj : scene) {
			Joint* joint = dynamic_cast<Joint*>(obj);
			if (joint && !joint->keyFrames.empty()) {
				const KeyFrame& firstKeyFrame = joint->keyFrames.front();
				joint->position = firstKeyFrame.position;
				joint->rotation = firstKeyFrame.rotation;
				joint->scale = firstKeyFrame.scale;
			}
		}

		// Reset playback frame range
		frame = frameBegin = 1;
		frameEnd = 0;
		for (auto& obj : scene) {
			Joint* joint = dynamic_cast<Joint*>(obj);
			if (joint && !joint->keyFrames.empty()) {
				frameEnd = std::max(frameEnd, joint->keyFrames.back().frame);
			}
		}

		bInPlayback = false;
		cout << "Scene loaded successfully!" << endl;
	}
	else {
		cout << "Load operation canceled." << endl;
	}
}



//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

	switch (key) {
	case OF_KEY_ALT:
		bAltKeyDown = false;
		mainCam.disableMouseInput();
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case 'x':
		bRotateX = false;
		break;
	case 'y':
		bRotateY = false;
		break;
	case 'z':
		bRotateZ = false;
		break;
	case 'a':
		bAnimate = false;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'C':
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case ' ':
		bInPlayback = !bInPlayback;
		break;
	case '.':
		nextFrame();
		break;
	case ',':
		prevFrame();
		break;
	case 'F':
	case 'a':
		bAnimate = true;
		break;
	case 'b':
		break;
	case 'd':
		resetKeyFrames();
		break;
	case 'f':
		ofToggleFullscreen();
		break;
	case 'h':
		bHide = !bHide;
		break;
	case 'i':
		break;
	case 'j':
		addJoint();
		break;
	case 'k':
		if (objSelected()) setKeyFrame();
		break;
	case 'l':
		loadFile();
		break;
	case 'n':
		break;
	case 'p':
		if (objSelected()) printChannels(selected[0]);
		break;
	case 'r':
		resetRotation();
		break;
	case 's':
		saveToFile();
		break;
	case 'x':
		bRotateX = true;
		break;
	case 'y':
		bRotateY = true;
		break;
	case 'z':
		bRotateZ = true;
		break;
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &topCam;
		break;
	case OF_KEY_ALT:
		bAltKeyDown = true;
		if (!mainCam.getMouseInputEnabled()) mainCam.enableMouseInput();
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_BACKSPACE:
		deleteObject();
		clearSelectionList();
		break;
	case OF_KEY_DEL:
		deleteKeyFrame();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	if (objSelected() && bDrag) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);
		if (bRotateX) {
			selected[0]->rotation += glm::vec3((point.x - lastPoint.x) * 20.0, 0, 0);
		}
		else if (bRotateY) {
			selected[0]->rotation += glm::vec3(0, (point.x - lastPoint.x) * 20.0, 0);
		}
		else if (bRotateZ) {
			selected[0]->rotation += glm::vec3(0, 0, (point.x - lastPoint.x) * 20.0);
		}
		else {
			selected[0]->position += (point - lastPoint);
		}
		lastPoint = point;
	}

}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3& point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

//--------------------------------------------------------------
//
// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
void ofApp::mousePressed(int x, int y, int button) {
	if (isMouseOverTimeline(x, y)) {
		// Check if we're clicking near any keyframe markers
		if (objSelected()) {
			Joint* selectedJoint = dynamic_cast<Joint*>(selected[0]);
			if (selectedJoint) {
				for (auto& kf : selectedJoint->keyFrames) {
					float kfX = ofMap(kf.frame, frameBegin, frameEnd, 10, timelineWidth + 10);
					float dist = glm::distance(glm::vec2(x, y), glm::vec2(kfX, timelineY + timelineHeight / 2));

					if (dist < keyframeMarkerSize) {
						// Right click to delete keyframe
						if (button == OF_MOUSE_BUTTON_RIGHT) {
							auto it = std::remove_if(selectedJoint->keyFrames.begin(), selectedJoint->keyFrames.end(),
								[&](const KeyFrame& k) { return k.frame == kf.frame; });
							selectedJoint->keyFrames.erase(it, selectedJoint->keyFrames.end());
							return;
						}
						// Left click to select frame
						else if (button == OF_MOUSE_BUTTON_LEFT) {
							frame = kf.frame;
							frameSlider = frame;
							return;
						}
					}
				}
			}
		}
		handleTimelineClick(x, y);
		return;
	}
	// if we are moving the camera around, don't allow selection
	//
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	if (!bCtrlKeyDown) {
		for (SceneObject* obj : selected) {
			obj->isSelected = false;
		}
		selected.clear();
	}

	//
	// test if something selected
	//
	vector<SceneObject*> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
		}
	}


	// if we selected more than one, pick nearest
	//
	SceneObject* selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else {
		selected.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bDrag = false;

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}