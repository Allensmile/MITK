/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#include "mitkTestingMacros.h"
#include "mitkEventConfig.h"
#include "mitkPropertyList.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionEventConst.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseWheelEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkMousePressEvent.h"
#include "mitkModule.h"
#include "mitkGetModuleContext.h"
#include <string>
#include <iostream>

int mitkEventConfigTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("EventConfig")

  /*
   * Loads a test a Config file and test if Config is build up correctly,
   * and if mapping from mitkEvents to EventVariant names works properly.
   * Indirectly this also tests the EventFactory Class, since we also test if the events have been constructed properly.
   *
   * The configuration object is constructed in three different ways,
   * each one is tested here.
   */


  // Construction using compiled-in resrouces:
   mitk::Module *module = mitk::GetModuleContext()->GetModule();
   mitk::EventConfig newConfig("StatemachineConfigTest.xml",module);

  MITK_TEST_CONDITION_REQUIRED(
         newConfig.IsValid() == true
          , "01 Check if file can be loaded and is valid" );
  /*
   * Test the global properties:
   * Test if stored values match the ones in the test config file.
   */
  mitk::PropertyList::Pointer properties = newConfig.GetAttributes();
  std::string prop1, prop2;
  MITK_TEST_CONDITION_REQUIRED(
        properties->GetStringProperty("property1",prop1) &&
        prop1 == "yes" &&
        properties->GetStringProperty("scrollModus",prop2) &&
        prop2 == "leftright"
        , "02 Check Global Properties");

  /*
   * Check if Events get mapped to the proper Variants
   */
  mitk::Point2D pos;
  mitk::MousePressEvent::Pointer mpe1 = mitk::MousePressEvent::New(NULL,pos,mitk::InteractionEvent::MiddleMouseButton | mitk::InteractionEvent::LeftMouseButton ,mitk::InteractionEvent::ControlKey | mitk::InteractionEvent::AltKey,mitk::InteractionEvent::LeftMouseButton  );
  mitk::MousePressEvent::Pointer standard1 = mitk::MousePressEvent::New(NULL,pos,mitk::InteractionEvent::LeftMouseButton,mitk::InteractionEvent::NoKey ,mitk::InteractionEvent::LeftMouseButton );
  mitk::MouseMoveEvent::Pointer mme1 = mitk::MouseMoveEvent::New(NULL,pos,mitk::InteractionEvent::RightMouseButton | mitk::InteractionEvent::LeftMouseButton,mitk::InteractionEvent::ShiftKey );
  mitk::MouseMoveEvent::Pointer mme2 = mitk::MouseMoveEvent::New(NULL,pos,mitk::InteractionEvent::RightMouseButton,mitk::InteractionEvent::ShiftKey );
  mitk::MouseWheelEvent::Pointer mwe1 = mitk::MouseWheelEvent::New(NULL,pos,mitk::InteractionEvent::RightMouseButton,mitk::InteractionEvent::ShiftKey,-2 );
  mitk::InteractionKeyEvent::Pointer ke = mitk::InteractionKeyEvent::New(NULL,"l",mitk::InteractionEvent::NoKey );


  MITK_TEST_CONDITION_REQUIRED(
        newConfig.GetMappedEvent(mpe1.GetPointer()) == "Variant1" &&
        newConfig.GetMappedEvent(standard1.GetPointer()) == "Standard1" &&
        newConfig.GetMappedEvent(mme1.GetPointer()) == "Move2" &&
        newConfig.GetMappedEvent(ke.GetPointer()) == "Key1" &&
        newConfig.GetMappedEvent(mme2.GetPointer()) == "" // does not exist in file
        , "03 Check Mouse- and Key-Events "  );


  // Construction providing a input stream

  std::ifstream* configStream = new std::ifstream("/MITK/MITK/Core/Code/Testing/Resources/Interactions/StatemachineConfigTest.xml");
  // if (myfile.is_open())
  newConfig(configStream);

  delete configStream;
  MITK_TEST_CONDITION_REQUIRED(
         newConfig.IsValid() == true
          , "01 Check if file can be loaded and is valid" );
  /*
   * Test the global properties:
   * Test if stored values match the ones in the test config file.
   */
  properties = newConfig.GetAttributes();
    MITK_TEST_CONDITION_REQUIRED(
        properties->GetStringProperty("property1",prop1) &&
        prop1 == "yes" &&
        properties->GetStringProperty("scrollModus",prop2) &&
        prop2 == "leftright"
        , "02 Check Global Properties");

  /*
   * Check if Events get mapped to the proper Variants
   */

  MITK_TEST_CONDITION_REQUIRED(
        newConfig.GetMappedEvent(mpe1.GetPointer()) == "Variant1" &&
        newConfig.GetMappedEvent(standard1.GetPointer()) == "Standard1" &&
        newConfig.GetMappedEvent(mme1.GetPointer()) == "Move2" &&
        newConfig.GetMappedEvent(ke.GetPointer()) == "Key1" &&
        newConfig.GetMappedEvent(mme2.GetPointer()) == "" // does not exist in file
        , "03 Check Mouse- and Key-Events "  );


  // always end with this!
  MITK_TEST_END()

}
