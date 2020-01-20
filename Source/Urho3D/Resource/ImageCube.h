//
// Copyright (c) 2020-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Graphics/GraphicsDefs.h"
#include "../Resource/Image.h"
#include "../Resource/Resource.h"
#include "../Math/SphericalHarmonics.h"

#include <EASTL/vector.h>

namespace Urho3D
{

class Deserializer;
class Image;
class XMLFile;

/// Cube texture resource.
class URHO3D_API ImageCube : public Resource
{
    URHO3D_OBJECT(ImageCube, Resource);

public:
    /// Construct.
    explicit ImageCube(Context* context);
    /// Destruct.
    ~ImageCube() override;
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;

    /// Return face images.
    const ea::vector<SharedPtr<Image>>& GetImages() const { return faceImages_; }

    /// Return parameters XML.
    XMLFile* GetParametersXML() const { return parametersXml_; }

    /// Get image data from a face's zero mip level. Only RGB and RGBA textures are supported.
    Image* GetImage(CubeMapFace face) const { return faceImages_[face]; }

    /// Return offset from the center of the unit cube for given texel (assuming zero mip level).
    Vector3 GetTexelOffsetVector(CubeMapFace face, int x, int y) const;
    /// Calculate spherical harmonics for the cube map.
    SphericalHarmonicsColor9 CalculateSphericalHarmonics() const;

private:
    /// Face images.
    ea::vector<SharedPtr<Image>> faceImages_;
    /// Parameter file.
    SharedPtr<XMLFile> parametersXml_;
    /// Cube width.
    int width_{};
};

}