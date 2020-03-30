// Copyright 2020 [Your Name]. All rights reserved.

#include <bayes/model.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <map>
#include <iostream>

std::vector<bayes::Image> image_list;

namespace bayes {
  const int kUnshadedIndex = 0;
  const int kShadedIndex = 1;

  // Laplace smoothing constant
  const double kLaplaceConstant = 5.0;
  const int kLaplaceDenomMultiplier = 2;

  std::ostream &operator<<(std::ostream &output, Model const &model) {
    std::string model_string = "";
    for (int i = 0; i < kNumClasses; i++) {
      model_string.append(std::to_string(model.priors_[i]));
      model_string.push_back('\n');
    }

    for (int i = 0; i < kImageSize; i++) {
      for (int j = 0; j < kImageSize; j++) {
        for (int c = 0; c < kNumClasses; c++) {
          for (int s = 0; s < kNumShades; s++) {
            model_string.append(std::to_string(i));
            model_string.push_back(' ');
            model_string.append(std::to_string(j));
            model_string.push_back(' ');
            model_string.append(std::to_string(c));
            model_string.push_back(' ');
            model_string.append(std::to_string(s));
            model_string.push_back(' ');
            model_string.append(std::to_string(model.probs_[i][j][c][s]));
            model_string.push_back('\n');
          }
        }
      }
    }

    output << model_string;
    return output;
  }

  void Model::CalculateProbabilities(std::string &labels_string) {
    std::map<int, int> class_occurrences;
    for (int i = 0; i < kNumClasses; i++) {
      class_occurrences.insert(std::make_pair(i, 0));
    }
    for (int i = 0; i < labels_string.size(); i++) {
      class_occurrences.find((int) (labels_string[i] - '0'))->second++;
    }
    for (int i = 0; i < kNumClasses; i++) {
      int a = class_occurrences.at(i);
      int len = labels_string.length();
      priors_[i] = ((double) class_occurrences.at(i) / (double) labels_string.length());
    }
    int shade_occurrences[kImageSize][kImageSize][kNumClasses]
                                        [kNumShades] = {0};
    for (int image = 0; image < image_list.size(); image++) {
      for (int i = 0; i < kImageSize; i++) {
        for (int j = 0; j < kImageSize; j++) {
          if (image_list[image].pixels_[i][j] == 1) {
            shade_occurrences[i][j][(int) labels_string[image] - '0']
            [kShadedIndex]++;
          } else /*if (image_list_[image].pixels_[i][j] == 0)*/ {
            shade_occurrences[i][j][(int) labels_string[image] - '0']
            [kUnshadedIndex]++;
          }
        }
      }
    }

    for (int i = 0; i < kImageSize; i++) {
      for (int j = 0; j < kImageSize; j++) {
        for (int c = 0; c < kNumClasses; c++) {
          for (int s = 0; s < kNumShades; s++) {
            double probability =
              (kLaplaceConstant + shade_occurrences[i][j][c][s])
              / (kLaplaceDenomMultiplier * kLaplaceConstant
                 + class_occurrences.at(c));
            probs_[i][j][c][s] = probability;
          }
        }
      }
    }
  }


}  // namespace bayes

bayes::Model::Model(std::istream &labels_file, std::istream &train_file) {
  std::string model_string;
  std::string train_line;
  while (std::getline(train_file, train_line)) {
    std::istringstream iss(train_line);
    model_string.append(iss.str());
  }
  if (model_string.length() > kImageSize * kImageSize) {
    for (int i = 0; i < model_string.length(); i += kImageSize * kImageSize) {
      Image image(model_string.substr(i, kImageSize * kImageSize));
      image_list.push_back(image);
    }
  }

  std::string labels_string;
  std::string label_line;
  while (std::getline(labels_file, label_line)) {
    std::istringstream iss(label_line);
    labels_string.append(iss.str());
  }

  CalculateProbabilities(labels_string);

  // Labels will have the same index as the contents of image list
}