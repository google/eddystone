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

///
/// Having a class that makes HTTP requests and sends results using callbacks seemes like a clean
/// implementation, since there are several classes that need to make HTTP requests.
///
class HTTPRequest {
  class func makeHTTPRequest(url: NSURL,
                       method: String,
                       postBody: NSData?,
                       requestHeaders: NSDictionary,
                       completionHandler: (httpResponseCode: Int,
                                           response: NSData?,
                                           error: NSError?) -> Void) {
    let request = NSMutableURLRequest(URL: url)
    for (key, value) in requestHeaders {
      request.addValue(value as! String, forHTTPHeaderField: key as! String)
    }
    request.HTTPMethod = method
    if let body = postBody {
      request.HTTPBody = body
    }
    let config = NSURLSessionConfiguration.defaultSessionConfiguration()
    config.timeoutIntervalForRequest = kRequestTimeout
    let session = NSURLSession(configuration: config)
    let task = session.dataTaskWithRequest(request, completionHandler: {(data, response, error) in
      let content = NSString(data: data!, encoding: NSUTF8StringEncoding)
      NSLog("CONTENT: \(content!)")
      NSLog("RESPONSE: \(response!)")
      NSLog("ERROR: \(error)")

      if let urlHTTPResponse = response as? NSHTTPURLResponse {
        let statusCode = urlHTTPResponse.statusCode
        completionHandler(httpResponseCode: statusCode, response: data, error: error)
      } else {
        completionHandler(httpResponseCode: -1, response: nil, error: error)
      }
    });
    task.resume()
  }

}
