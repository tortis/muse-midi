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
#include "portcls.h"
#include "stdunk.h"

/*****************************************************************************
 * CUnknown implementation
 */

/*****************************************************************************
 * CUnknown::CUnknown()
 *****************************************************************************
 * Constructor.
 */
CUnknown::CUnknown(PUNKNOWN pUnknownOuter)
:   m_lRefCount(0)
{
    if (pUnknownOuter)
    {
        m_pUnknownOuter = pUnknownOuter;
    }
    else
    {
        m_pUnknownOuter = PUNKNOWN(dynamic_cast<PNONDELEGATINGUNKNOWN>(this));
    }
}

/*****************************************************************************
 * CUnknown::~CUnknown()
 *****************************************************************************
 * Destructor.
 */
CUnknown::~CUnknown(void)
{
}

/*****************************************************************************
 * INonDelegatingUnknown implementation
 */

/*****************************************************************************
 * CUnknown::NonDelegatingAddRef()
 *****************************************************************************
 * Register a new reference to the object without delegating to the outer
 * unknown.
 */
STDMETHODIMP_(ULONG) CUnknown::NonDelegatingAddRef(void)
{
    ASSERT(m_lRefCount >= 0);

    InterlockedIncrement(&m_lRefCount);

    return ULONG(m_lRefCount);
}

/*****************************************************************************
 * CUnknown::NonDelegatingRelease()
 *****************************************************************************
 * Release a reference to the object without delegating to the outer unknown.
 */
STDMETHODIMP_(ULONG) CUnknown::NonDelegatingRelease(void)
{
    ASSERT(m_lRefCount > 0);

    if (InterlockedDecrement(&m_lRefCount) == 0)
    {
        m_lRefCount++;
        delete this;
        return 0;
    }

    return ULONG(m_lRefCount); 
}

/*****************************************************************************
 * CUnknown::NonDelegatingQueryInterface()
 *****************************************************************************
 * Obtains an interface.
 */
STDMETHODIMP_(NTSTATUS) CUnknown::NonDelegatingQueryInterface
(
    _In_ REFIID  rIID,
    _COM_Outptr_ PVOID * ppVoid
)
{
    ASSERT(ppVoid);

    if (IsEqualGUIDAligned(rIID,IID_IUnknown))
    {
        *ppVoid = PVOID(PUNKNOWN(this));
    }
    else
    {
        *ppVoid = NULL;
    }
    
    if (*ppVoid)
    {
        PUNKNOWN(*ppVoid)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

