// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_INFOBARS_MODALS_INFOBAR_PASSWORD_TABLE_VIEW_CONTROLLER_H_
#define IOS_CHROME_BROWSER_UI_INFOBARS_MODALS_INFOBAR_PASSWORD_TABLE_VIEW_CONTROLLER_H_

#import "ios/chrome/browser/ui/table_view/chrome_table_view_controller.h"

@protocol InfobarModalDelegate;

// InfobarPasswordTableViewController represents the content for the Passwords
// InfobarModal.
@interface InfobarPasswordTableViewController : ChromeTableViewController

// InfobarModalDelegate for this ViewController.
@property(nonatomic, strong) id<InfobarModalDelegate> infobarModalDelegate;
// The username being displayed in the InfobarModal.
@property(nonatomic, copy) NSString* username;
// The URL being displayed in the InfobarModal.
@property(nonatomic, copy) NSString* URL;

@end

#endif  // IOS_CHROME_BROWSER_UI_INFOBARS_MODALS_INFOBAR_PASSWORD_TABLE_VIEW_CONTROLLER_H_
