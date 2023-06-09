#include "preprocessor.h"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <iostream>
//#include <cassert>
#define MAT(mat) //cout << #mat ": rows " << mat.rows << ", cols " << mat.cols << ", type " << mat.type() << ", bytes " <<  mat.total()*mat.elemSize() << endl;

Preprocessor::Preprocessor(const int max_num_keypoints){    
    orb_params * orb_params_ = new orb_params("default");        
    extractor = new orb_extractor(orb_params_, max_num_keypoints, {});
    cout << "extractor " << extractor << endl;
}

cv::Mat Preprocessor::to8UC4Mat(int ptr, int width, int height){
    //assert(array.size() == width * height * 4);
    return Mat(height, width, CV_8UC4, reinterpret_cast<void*>(ptr));
}

val Preprocessor::toArray(cv::Mat mat){
    //cout << "toArray" << endl;
    //MAT(mat)
    val result = val::object();
    result.set("width", mat.cols);
    result.set("height", mat.rows);
    result.set("type", mat.type()); // CvMat type
    result.set("elementSize", mat.elemSize());  // Size in bytes
    result.set("array", typed_memory_view(mat.total()*mat.elemSize(), mat.data));   // elemSize is 4 for RGBA
    //cout << "toArray ending " << endl;
    return result;
}

val Preprocessor::preprocess(int ptr, int width, int height){
    //cout << "preprocess " << ptr << ", " << width << ", " << height << endl;
    Mat img = to8UC4Mat(ptr, width, height);
    //MAT(img)
    cvtColor(img, imGray, cv::COLOR_RGBA2GRAY); // RGBA is the format used in javascript
    //MAT(imGray)
    Mat descriptors;
    extractor->extract(imGray, Mat(), keypoints, descriptors);
    //cout << "extract " << keypoints.size() << endl;
    MAT(descriptors)

    // serialize descriptors and keypoints on a Mat
    auto howManyFeatures = descriptors.rows;  // Same as keypoints.size()
    //cout << "howManyFeatures " << howManyFeatures << endl;
    Mat serializedFeatures(howManyFeatures*2, 4, CV_32FC1, descriptors.data);
    serializedFeatures.resize(howManyFeatures*3);  // Possible copy of descriptors
    size_t row = howManyFeatures*2;//, end=serializedFeatures.rows;
    for(const auto& keypoint : keypoints){
        serializedFeatures.at<float>(row, 0) = keypoint.pt.x;
        serializedFeatures.at<float>(row, 1) = keypoint.pt.y;
        serializedFeatures.at<float>(row, 2) = keypoint.angle;
        serializedFeatures.at<float>(row, 3) = *(float*)&keypoint.octave;
        row++;
    }

    /*
    serializedFeatures.reserve(howManyFeatures*3);  // Possible copy of descriptors
    // Add keypoints
    for(const auto& keypoint : keypoints){
        serializedFeatures.push_back((Mat_<float>(1,4)<<
            keypoint.pt.x,
            keypoint.pt.y,
            keypoint.angle,
            *(float*)&keypoint.octave
        ));
    }*/
    /*
    vector<float> feature(4);
    cout << " feature.size " << feature.size() << endl;
    for(const auto& keypoint : keypoints){
        cout << "<";
        feature[0] = keypoint.pt.x;
        feature[1] = keypoint.pt.y;
        feature[2] = keypoint.angle;
        feature[3] = *(float*)&keypoint.octave;
        cout << "-";
        serializedFeatures.push_back(Mat(feature));
        cout << ">";
    }
    */
    //cout << endl;
    //cout << "After adding keypoints ";
    MAT(serializedFeatures)
    return toArray(serializedFeatures);
}

val Preprocessor::getAnnotations(){
    //cout << "getAnnotations ";
    MAT(imGray)

    if (imGray.empty())
        return val(0);

    Mat annotatedImage;
    cvtColor(imGray, annotatedImage, cv::COLOR_GRAY2RGBA); // RGBA is the format used in javascript
    MAT(annotatedImage)

    // drawkeypoints works on RGBA too
    drawKeypoints(annotatedImage, keypoints, annotatedImage, Scalar(0,255,0,0));
    //cout << "drawKeypoints ";
    return toArray(annotatedImage);
}