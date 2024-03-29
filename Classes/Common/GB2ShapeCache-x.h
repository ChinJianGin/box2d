﻿/*
MIT License

Loads physics sprites created with http://www.PhysicsEditor.de
To be used with cocos2d-x

Generic Shape Cache for box2d

Created by Thomas Broquist

Copyright (c) 2010 Andreas Loew / www.code-and-web.de
Copyright (c) 2012 Chris Hannon / channon.us
Copyright (c) 2013 Tomoaki Shimizu / tks2.net

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef GB2ShapeCache_x_h
#define GB2ShapeCache_x_h

#include "cocos2d.h"

class BodyDef;
class b2Body;

using namespace cocos2d;

namespace gbox2d {

	/**
	* The class used for loading physics body details from a *.plist generated by
	* PhysicsEditor and storing them in a cache for later generating bodies
	*/
	class GB2ShapeCache {
	public:
		// Static interface
		static GB2ShapeCache* getInstance(void);

	public:
		/**
		* Intialize the class and properties to their defaults
		* @return true if successful
		*/
		bool init();

		/**
		* Load shapes stored in the plist specified
		* @param plist the filename of the plist
		*/
		void addShapesWithFile(const std::string &plist);

		/**
		* Add fixtures for the specified shape to the given body
		* @param body the body
		* @param shape the shape name
		*/
		void addFixturesToBody(b2Body *body, const std::string &shape);

		/**
		* Returns the anchor point for the shape with the name given
		* @param shape the name of the shape
		* @return the anchor point
		*/
		cocos2d::Point anchorPointForShape(const std::string &shape);

		/**
		* Reset the cache
		*/
		void reset();

		/**
		* Gets the pixel to meter ratio
		* @return the Pixel to meter ratio
		*/
		float getPtmRatio() { return ptmRatio; }

		~GB2ShapeCache() {}

	private:
		std::map<std::string, BodyDef *> shapeObjects; //!< the map of shape objects loaded from the plist
		GB2ShapeCache(void) {}
		float ptmRatio; //!< the pixel to meter ratio
	};

}

#endif