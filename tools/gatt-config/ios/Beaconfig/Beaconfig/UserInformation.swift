// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import Foundation
import UIKit

let kGetProjectsServer = "https://cloudresourcemanager.googleapis.com/v1beta1/projects"

enum GettingProjectsListState {
  case UserHasNoProjects
  case UnableToGetProjects
  case GotUserProjects
}

///
/// Using Storyboard segues to load the UserLoginViewController was a safe choice, but segues
/// create a new instance of the view controller every time the segue is triggered. We needed
/// a class to hold the state of the view controller.
///
class UserInformation {
  var userImage: UIImage
  var userName: String?
  var userEmail: String?
  var userProjects: NSMutableArray?
  var statusInfo: String
  var userCurrentlySignedIn: Bool
  var projectsArray: NSMutableArray!
  var gotProjectsListCallback: ((state: GettingProjectsListState) -> Void)?
  var selectedProjectIndex: Int?

  init() {
    userImage = UIImage(named: "Google")!
    statusInfo = "Currently not signed in"
    userCurrentlySignedIn = false
    projectsArray = ["Not Selected"]
  }

  /// We want to clear all the data we have when the user signs out.
  func clearData() {
    userImage = UIImage(named: "Google")!
    statusInfo = "Currently not signed in"
    userCurrentlySignedIn = false
    projectsArray = ["Not Selected"]
    selectedProjectIndex = nil
  }

  func parseUserProjects(data: NSData) {
    do {
      let json = try NSJSONSerialization.JSONObjectWithData(data, options: .AllowFragments)
      if let projectsList = json["projects"] as? NSArray {
        for project in projectsList {
          if let
            projectID = project["projectId"],
            ID = projectID as? String {
            projectsArray.addObject(ID)
          }
        }
        if let callback = gotProjectsListCallback {
          callback(state: GettingProjectsListState.GotUserProjects)
        }
      } else {
        /// The user has no projects.
        if let callback = gotProjectsListCallback {
          callback(state: GettingProjectsListState.UserHasNoProjects)
        }
      }
    } catch {
      print("error serializing JSON: \(error)")
    }
  }

  func getprojectListHTTPRequest() {
    if GIDSignIn.sharedInstance().currentUser != nil {
      let bearer = GIDSignIn.sharedInstance().currentUser.authentication.accessToken
      let bearerHeader = "Bearer \(bearer)"
      let server = kGetProjectsServer
      let url = NSURL(string: server)
      let httpHeaders = ["Authorization" : bearerHeader,
                         "Accept" : "application/json"]
      if let requestURL = url {
        HTTPRequest.makeHTTPRequest(requestURL,
                                    method: "GET",
                                    postBody: nil,
                                    requestHeaders: httpHeaders) { statusCode, data, error in
                                      if statusCode == 200 {
                                        self.parseUserProjects(data!)
                                      } else {
                                        if let callback = self.gotProjectsListCallback {
                                          callback(state:
                                                    GettingProjectsListState.UnableToGetProjects)
                                        }
                                      }

        }
      }
    }
  }

  func getProjectList(callback: (state: GettingProjectsListState) -> Void) {
    gotProjectsListCallback = callback
    getprojectListHTTPRequest()
  }

}
