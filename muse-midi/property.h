/*
  ==============================================================================
   This file is part of the Springbeats Virtual MIDI Driver (sbvmidi).
   Copyright (c) 2016 - Springbeats SARL.
   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3
   Details of these licenses can be found at: www.gnu.org/licenses
   Sbvmidi is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
   ------------------------------------------------------------------------------
   To release a closed-source product which uses sbvmidi, commercial licenses are
   available: visit www.springbeats.com for more information.
  ==============================================================================
*/
#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#define INITGUID 1
#include "portcls.h"
#include <ntstrsafe.h>
#include "stdunk.h"
#include "dmusicks.h"
#include "musemidi.h"

/*****************************************************************************
* PinDataRangesStreamLegacy
* PinDataRangesStreamDMusic
*****************************************************************************
* Structures indicating range of valid format values for live pins.
*/
static KSDATARANGE_MUSIC PinDataRangesStreamLegacy =
{
    {
        sizeof(KSDATARANGE_MUSIC),
        0,
        0,
        0,
        STATICGUIDOF(KSDATAFORMAT_TYPE_MUSIC),
        STATICGUIDOF(KSDATAFORMAT_SUBTYPE_MIDI),
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
    },
    STATICGUIDOF(KSMUSIC_TECHNOLOGY_PORT),
    0,
    0,
    0xFFFF
};

static KSDATARANGE_MUSIC PinDataRangesStreamDMusic =
{
    {
        sizeof(KSDATARANGE_MUSIC),
        0,
        0,
        0,
        STATICGUIDOF(KSDATAFORMAT_TYPE_MUSIC),
        STATICGUIDOF(KSDATAFORMAT_SUBTYPE_DIRECTMUSIC),
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
    },
    STATICGUIDOF(KSMUSIC_TECHNOLOGY_PORT),
    0,
    0,
    0xFFFF
};

/*****************************************************************************
* PinDataRangePointersStreamLegacy
* PinDataRangePointersStreamDMusic
* PinDataRangePointersStreamCombined
*****************************************************************************
* List of pointers to structures indicating range of valid format values
* for live pins.
*/
static PKSDATARANGE PinDataRangePointersStreamLegacy[] =
{
    PKSDATARANGE(&PinDataRangesStreamLegacy)
};

static PKSDATARANGE PinDataRangePointersStreamDMusic[] =
{
    PKSDATARANGE(&PinDataRangesStreamDMusic)
};

static PKSDATARANGE PinDataRangePointersStreamCombined[] =
{
    PKSDATARANGE(&PinDataRangesStreamLegacy),
    PKSDATARANGE(&PinDataRangesStreamDMusic)
};

/*****************************************************************************
* PinDataRangesBridge
*****************************************************************************
* Structures indicating range of valid format values for bridge pins.
*/
static KSDATARANGE PinDataRangesBridge[] =
{
    {
        sizeof(KSDATARANGE),
        0,
        0,
        0,
        STATICGUIDOF(KSDATAFORMAT_TYPE_MUSIC),
        STATICGUIDOF(KSDATAFORMAT_SUBTYPE_MIDI_BUS),
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
    }
};

/*****************************************************************************
* PinDataRangePointersBridge
*****************************************************************************
* List of pointers to structures indicating range of valid format values
* for bridge pins.
*/
static PKSDATARANGE PinDataRangePointersBridge[] =
{
    &PinDataRangesBridge[0]
};

/*****************************************************************************
* SynthProperties
*****************************************************************************
* List of properties in the Synth set.
*/
static PCPROPERTY_ITEM SynthProperties[] =
{
    // Global: S/Get synthesizer caps
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_CAPS,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Synth
    },
    // Global: S/Get port parameters
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_PORTPARAMETERS,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Synth
    },
    // Per stream: S/Get channel groups
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_CHANNELGROUPS,
        KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Synth
    },
    // Per stream: Get current latency time
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_LATENCYCLOCK,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_Synth
    }
};

DEFINE_PCAUTOMATION_TABLE_PROP(AutomationSynth, SynthProperties);
DEFINE_PCAUTOMATION_TABLE_PROP(AutomationSynth2, SynthProperties);

#define kMaxNumCaptureStreams       1
#define kMaxNumLegacyRenderStreams  1
#define kMaxNumDMusicRenderStreams  1

/*****************************************************************************
* MiniportPins
*****************************************************************************
* List of pins.
*/
static PCPIN_DESCRIPTOR MiniportPins[] =
{
    {
        kMaxNumLegacyRenderStreams, kMaxNumLegacyRenderStreams, 0,    // InstanceCount
        NULL,                                                       // AutomationTable
        {                                                           // KsPinDescriptor
            0,                                              // InterfacesCount
            NULL,                                           // Interfaces
            0,                                              // MediumsCount
            NULL,                                           // Mediums
            SIZEOF_ARRAY(PinDataRangePointersStreamLegacy), // DataRangesCount
            PinDataRangePointersStreamLegacy,               // DataRanges
            KSPIN_DATAFLOW_IN,                              // DataFlow
            KSPIN_COMMUNICATION_SINK,                       // Communication
            (GUID *)&KSCATEGORY_AUDIO,                     // Category
            &KSAUDFNAME_MIDI,                               // Name
            0                                               // Reserved
        }
    },
    {
        kMaxNumDMusicRenderStreams, kMaxNumDMusicRenderStreams, 0,    // InstanceCount
        NULL,                                                       // AutomationTable
        {                                                           // KsPinDescriptor
            0,                                              // InterfacesCount
            NULL,                                           // Interfaces
            0,                                              // MediumsCount
            NULL,                                           // Mediums
            SIZEOF_ARRAY(PinDataRangePointersStreamDMusic), // DataRangesCount
            PinDataRangePointersStreamDMusic,               // DataRanges
            KSPIN_DATAFLOW_IN,                              // DataFlow
            KSPIN_COMMUNICATION_SINK,                       // Communication
            (GUID *)&KSCATEGORY_AUDIO,                     // Category
            &KSAUDFNAME_DMUSIC_MPU_OUT,                     // Name
            0                                               // Reserved
        }
    },
    {
        0, 0, 0,                                      // InstanceCount
        NULL,                                       // AutomationTable
        {                                           // KsPinDescriptor
            0,                                          // InterfacesCount
            NULL,                                       // Interfaces
            0,                                          // MediumsCount
            NULL,                                       // Mediums
            SIZEOF_ARRAY(PinDataRangePointersBridge),   // DataRangesCount
            PinDataRangePointersBridge,                 // DataRanges
            KSPIN_DATAFLOW_OUT,                         // DataFlow
            KSPIN_COMMUNICATION_NONE,                   // Communication
            (GUID *)&KSCATEGORY_AUDIO,                 // Category
            NULL,                                       // Name
            0                                           // Reserved
        }
    },
    {
        0, 0, 0,                                      // InstanceCount
        NULL,                                       // AutomationTable
        {                                           // KsPinDescriptor
            0,                                          // InterfacesCount
            NULL,                                       // Interfaces
            0,                                          // MediumsCount
            NULL,                                       // Mediums
            SIZEOF_ARRAY(PinDataRangePointersBridge),   // DataRangesCount
            PinDataRangePointersBridge,                 // DataRanges
            KSPIN_DATAFLOW_IN,                          // DataFlow
            KSPIN_COMMUNICATION_NONE,                   // Communication
            (GUID *)&KSCATEGORY_AUDIO,                 // Category
            NULL,                                       // Name
            0                                           // Reserved
        }
    },
    {
        kMaxNumCaptureStreams, kMaxNumCaptureStreams, 0,      // InstanceCount
        NULL,                                               // AutomationTable
        {                                                   // KsPinDescriptor
            0,                                                // InterfacesCount
            NULL,                                             // Interfaces
            0,                                                // MediumsCount
            NULL,                                             // Mediums
            SIZEOF_ARRAY(PinDataRangePointersStreamCombined), // DataRangesCount
            PinDataRangePointersStreamCombined,               // DataRanges
            KSPIN_DATAFLOW_OUT,                               // DataFlow
            KSPIN_COMMUNICATION_SINK,                         // Communication
            (GUID *)&KSCATEGORY_AUDIO,                       // Category
            &KSAUDFNAME_DMUSIC_MPU_IN,                        // Name
            0                                                 // Reserved
        }
    }
};

/*****************************************************************************
* MiniportNodes
*****************************************************************************
* List of nodes.
*/
#define CONST_PCNODE_DESCRIPTOR(n)          { 0, NULL, &n, NULL }
#define CONST_PCNODE_DESCRIPTOR_AUTO(n,a)   { 0, &a, &n, NULL }

static PCNODE_DESCRIPTOR MiniportNodes[] =
{
    CONST_PCNODE_DESCRIPTOR_AUTO(KSNODETYPE_SYNTHESIZER, AutomationSynth),
    CONST_PCNODE_DESCRIPTOR_AUTO(KSNODETYPE_SYNTHESIZER, AutomationSynth2)
};

/*****************************************************************************
* MiniportConnections
*****************************************************************************
* List of connections.
*/
enum
{
    eSynthNode = 0,
    eInputNode
};

enum
{
    eFilterInputPinLeg = 0,
    eFilterInputPinDM,
    eBridgeOutputPin,
    eBridgeInputPin,
    eFilterOutputPin
};

static PCCONNECTION_DESCRIPTOR MiniportConnections[] =
{  // From                                   To
    // Node           pin                     Node           pin
    { PCFILTER_NODE, eFilterInputPinLeg, PCFILTER_NODE, eBridgeOutputPin }      // Legacy Stream in to synth.
    , { PCFILTER_NODE, eFilterInputPinDM, eSynthNode, KSNODEPIN_STANDARD_IN } // DM Stream in to synth.
    , { eSynthNode, KSNODEPIN_STANDARD_OUT, PCFILTER_NODE, eBridgeOutputPin }      // Synth to bridge out.
    , { PCFILTER_NODE, eBridgeInputPin, eInputNode, KSNODEPIN_STANDARD_IN } // Bridge in to input.
    , { eInputNode, KSNODEPIN_STANDARD_OUT, PCFILTER_NODE, eFilterOutputPin }      // Input to DM/Legacy Stream out.
};

/*****************************************************************************
* MiniportCategories
*****************************************************************************
* List of categories.
*/
static GUID MiniportCategories[] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_RENDER),
    STATICGUIDOF(KSCATEGORY_CAPTURE)
};

/*****************************************************************************
* MiniportFilterDescriptor
*****************************************************************************
* Complete miniport filter description.
*/
static PCFILTER_DESCRIPTOR MiniportFilterDescriptor =
{
    0,                                  // Version
    NULL,                               // AutomationTable
    sizeof(PCPIN_DESCRIPTOR),           // PinSize
    SIZEOF_ARRAY(MiniportPins),         // PinCount
    MiniportPins,                       // Pins
    sizeof(PCNODE_DESCRIPTOR),          // NodeSize
    SIZEOF_ARRAY(MiniportNodes),        // NodeCount
    MiniportNodes,                      // Nodes
    SIZEOF_ARRAY(MiniportConnections),  // ConnectionCount
    MiniportConnections,                // Connections
    SIZEOF_ARRAY(MiniportCategories),   // CategoryCount
    MiniportCategories                  // Categories
};

#endif // _PROPERTY_H_
