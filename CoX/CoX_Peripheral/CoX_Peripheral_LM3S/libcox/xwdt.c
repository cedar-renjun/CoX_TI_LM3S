//*****************************************************************************
//
//! \file xwdt.c
//! \brief Driver for the Watchdog Timer
//! \version V2.1.1.1
//! \date 07/15/2012
//! \author CooCox
//! \copy
//!
//! Copyright (c)  2012, CooCox
//! All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!     * Redistributions of source code must retain the above copyright
//! notice, this list of conditions and the following disclaimer.
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution.
//!     * Neither the name of the <ORGANIZATION> nor the names of its
//! contributors may be used to endorse or promote products derived
//! from this software without specific prior written permission.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup watchdog_api
//! @{
//
//*****************************************************************************

#include "xhw_ints.h"
#include "xhw_memmap.h"
#include "xhw_types.h"
#include "xhw_wdt.h"
#include "xdebug.h"
#include "xwdt.h"


typedef struct
{
    unsigned long   Base;
    xtEventCallback Fun;
}tWdtFun;

static tWdtFun g_CallBackTbl[] =
{
    {
        WATCHDOG0_BASE,
        (xtEventCallback) 0,
    },
    {
        WATCHDOG1_BASE,
        (xtEventCallback) 0,
    },
};


void Watchdog1WriteSync(void)
{
    while(!(xHWREG(WATCHDOG1_BASE + WDT_O_CTL) & WDT_CTL_WRC));
}

void WDTIntHandler(void)
{
    unsigned long i = 0;
    unsigned long IntStatus = 0;
    unsigned long TblSize = sizeof(g_CallBackTbl) / sizeof(g_CallBackTbl[0]);

    for(i = 0; i < TblSize; i++)
    {
        IntStatus = xHWREG(g_CallBackTbl[i].Base + WDT_O_MIS);
        if((IntStatus != 0) && (g_CallBackTbl[i].Fun != 0))
        {
            //
            // Clear the Flag
            //
            WatchdogIntClear(g_CallBackTbl[i].Base);

            //
            // Call User's Function
            //
            g_CallBackTbl[i].Fun(0,0,0,0);
        }
    }
}


//*****************************************************************************
//
//! Determines if the watchdog timer is enabled.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function checks to see if the watchdog timer is enabled.
//!
//! \return Returns \b xtrue if the watchdog timer is enabled and \b xfalse
//! if it is not.
//
//*****************************************************************************
xtBoolean WatchdogRunning(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // See if the watchdog timer module is enabled, and return.
    //
    return(xHWREG(ulBase + WDT_O_CTL) & WDT_CTL_INTEN);
}

//*****************************************************************************
//
//! Enables the watchdog timer.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function enables the watchdog timer counter and interrupt.
//!
//! \note This function has no effect if the watchdog timer has been locked.
//!
//! \sa WatchdogLock(), WatchdogUnlock()
//!
//! \return None.
//
//*****************************************************************************
void WatchdogEnable(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Enable the watchdog timer module.
    //
    xHWREG(ulBase + WDT_O_CTL) |= WDT_CTL_INTEN;
}

//*****************************************************************************
//
//! Enables the watchdog timer reset.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function enables the capability of the watchdog timer to issue a reset
//! to the processor after a second timeout condition.
//!
//! \note This function has no effect if the watchdog timer has been locked.
//!
//! \sa WatchdogLock(), WatchdogUnlock()
//!
//! \return None.
//
//*****************************************************************************
void WatchdogResetEnable(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Enable the watchdog reset.
    //

    xHWREG(ulBase + WDT_O_CTL) |= WDT_CTL_RESEN;
}

//*****************************************************************************
//
//! Disables the watchdog timer reset.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function disables the capability of the watchdog timer to issue a
//! reset to the processor after a second timeout condition.
//!
//! \note This function has no effect if the watchdog timer has been locked.
//!
//! \sa WatchdogLock(), WatchdogUnlock()
//!
//! \return None.
//
//*****************************************************************************
void WatchdogResetDisable(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Disable the watchdog reset.
    //

    xHWREG(ulBase + WDT_O_CTL) &= ~(WDT_CTL_RESEN);
}

//*****************************************************************************
//
//! Enables the watchdog timer lock mechanism.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function locks out write access to the watchdog timer configuration
//! registers.
//!
//! \return None.
//
//*****************************************************************************
void WatchdogLock(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Lock out watchdog register writes.  Writing anything to the WDT_O_LOCK
    // register causes the lock to go into effect.
    //
    xHWREG(ulBase + WDT_O_LOCK) = WDT_LOCK_LOCKED;
}

//*****************************************************************************
//
//! Disables the watchdog timer lock mechanism.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function enables write access to the watchdog timer configuration
//! registers.
//!
//! \return None.
//
//*****************************************************************************
void WatchdogUnlock(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Unlock watchdog register writes.
    //
    xHWREG(ulBase + WDT_O_LOCK) = WDT_LOCK_UNLOCK;
}

//*****************************************************************************
//
//! Gets the state of the watchdog timer lock mechanism.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function returns the lock state of the watchdog timer registers.
//!
//! \return Returns \b xtrue if the watchdog timer registers are locked, and
//! \b xfalse if they are not locked.
//
//*****************************************************************************
xtBoolean WatchdogLockState(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Get the lock state.
    //
    return((xHWREG(ulBase + WDT_O_LOCK) == WDT_LOCK_LOCKED) ? xtrue : xfalse);
}

//*****************************************************************************
//
//! Sets the watchdog timer reload value.
//!
//! \param ulBase is the base address of the watchdog timer module.
//! \param ulLoadVal is the load value for the watchdog timer.
//!
//! This function configures the value to load into the watchdog timer when the
//! count reaches zero for the first time; if the watchdog timer is running
//! when this function is called, then the value is immediately loaded into the
//! watchdog timer counter.  If the \e ulLoadVal parameter is 0, then an
//! interrupt is immediately generated.
//!
//! \note This function has no effect if the watchdog timer has been locked.
//!
//! \sa WatchdogLock(), WatchdogUnlock(), WatchdogReloadGet()
//!
//! \return None.
//
//*****************************************************************************
void WatchdogReloadSet(unsigned long ulBase, unsigned long ulLoadVal)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Set the load register.
    //
    xHWREG(ulBase + WDT_O_LOAD) = ulLoadVal;
}

//*****************************************************************************
//
//! Gets the watchdog timer reload value.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function gets the value that is loaded into the watchdog timer when
//! the count reaches zero for the first time.
//!
//! \sa WatchdogReloadSet()
//!
//! \return None.
//
//*****************************************************************************
unsigned long WatchdogReloadGet(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Get the load register.
    //
    return(xHWREG(ulBase + WDT_O_LOAD));
}

//*****************************************************************************
//
//! Gets the current watchdog timer value.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function reads the current value of the watchdog timer.
//!
//! \return Returns the current value of the watchdog timer.
//
//*****************************************************************************
unsigned long WatchdogValueGet(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Get the current watchdog timer register value.
    //
    return(xHWREG(ulBase + WDT_O_VALUE));
}

//*****************************************************************************
//
//! Enables the watchdog timer interrupt.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function enables the watchdog timer interrupt.
//!
//! \note This function has no effect if the watchdog timer has been locked.
//!
//! \sa WatchdogLock(), WatchdogUnlock(), WatchdogEnable()
//!
//! \return None.
//
//*****************************************************************************
void WatchdogIntEnable(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Enable the watchdog interrupt.
    //
    xHWREG(ulBase + WDT_O_CTL) |= WDT_CTL_INTEN;
}

//*****************************************************************************
//
//! Gets the current watchdog timer interrupt status.
//!
//! \param ulBase is the base address of the watchdog timer module.
//! \param bMasked is \b xfalse if the raw interrupt status is required and
//! \b xtrue if the masked interrupt status is required.
//!
//! This function returns the interrupt status for the watchdog timer module.
//! Either the raw interrupt status or the status of interrupt that is allowed
//! to reflect to the processor can be returned.
//!
//! \return Returns the current interrupt status, where a 1 indicates that the
//! watchdog interrupt is active, and a 0 indicates that it is not active.
//
//*****************************************************************************
unsigned long WatchdogIntStatus(unsigned long ulBase, xtBoolean bMasked)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Return either the interrupt status or the raw interrupt status as
    // requested.
    //
    if(bMasked)
    {
        return(xHWREG(ulBase + WDT_O_MIS));
    }
    else
    {
        return(xHWREG(ulBase + WDT_O_RIS));
    }
}

//*****************************************************************************
//
//! Clears the watchdog timer interrupt.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! The watchdog timer interrupt source is cleared, so that it no longer
//! asserts.
//!
//! \note Because there is a write buffer in the Cortex-M processor, it may
//! take several clock cycles before the interrupt source is actually cleared.
//! Therefore, it is recommended that the interrupt source be cleared early in
//! the interrupt handler (as opposed to the very last action) to avoid
//! returning from the interrupt handler before the interrupt source is
//! actually cleared.  Failure to do so may result in the interrupt handler
//! being immediately reentered (because the interrupt controller still sees
//! the interrupt source asserted).
//!
//! \return None.
//
//*****************************************************************************
void WatchdogIntClear(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Clear the interrupt source.
    //
    xHWREG(ulBase + WDT_O_ICR) = WDT_INT_TIMEOUT;
}

//*****************************************************************************
//
//! Sets the type of interrupt generated by the watchdog.
//!
//! \param ulBase is the base address of the watchdog timer module.
//! \param ulType is the type of interrupt to generate.
//!
//! This function sets the type of interrupt that is generated if the watchdog
//! timer expires.  \e ulType can be either \b WATCHDOG_INT_TYPE_INT to
//! generate a standard interrupt (the default) or \b WATCHDOG_INT_TYPE_NMI to
//! generate a non-maskable interrupt (NMI).
//!
//! When configured to generate an NMI, the watchdog interrupt must still be
//! enabled with WatchdogIntEnable(), and it must still be cleared inside the
//! NMI handler with WatchdogIntClear().
//!
//! \note The ability to select an NMI interrupt varies with the Stellaris part
//! in use.  Please consult the datasheet for the part you are using to
//! determine whether this support is available.
//!
//! \return None.
//
//*****************************************************************************
void WatchdogIntTypeSet(unsigned long ulBase, unsigned long ulType)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));
    xASSERT((ulType == WATCHDOG_INT_TYPE_INT) ||
           (ulType == WATCHDOG_INT_TYPE_NMI));

    //
    // Set the interrupt type.
    //
    xHWREG(ulBase + WDT_O_CTL) =
        (xHWREG(ulBase + WDT_O_CTL) & ~WDT_CTL_INTTYPE) | ulType;
}

//*****************************************************************************
//
//! Enables stalling of the watchdog timer during debug events.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function allows the watchdog timer to stop counting when the processor
//! is stopped by the debugger.  By doing so, the watchdog is prevented from
//! expiring (typically almost immediately from a human time perspective) and
//! resetting the system (if reset is enabled).  The watchdog instead expires
//! after the appropriate number of processor cycles have been executed while
//! debugging (or at the appropriate time after the processor has been
//! restarted).
//!
//! \return None.
//
//*****************************************************************************
void WatchdogStallEnable(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Enable timer stalling.
    //
    xHWREG(ulBase + WDT_O_TEST) |= WDT_TEST_STALL;
}

//*****************************************************************************
//
//! Disables stalling of the watchdog timer during debug events.
//!
//! \param ulBase is the base address of the watchdog timer module.
//!
//! This function disables the debug mode stall of the watchdog timer.  By
//! doing so, the watchdog timer continues to count regardless of the processor
//! debug state.
//!
//! \return None.
//
//*****************************************************************************
void WatchdogStallDisable(unsigned long ulBase)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    //
    // Disable timer stalling.
    //
    xHWREG(ulBase + WDT_O_TEST) &= ~(WDT_TEST_STALL);
}



void xWDTInit(unsigned long ulBase,
              unsigned long ulConfig,
              unsigned long ulReload)
{
    //
    // Check the arguments.
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));
    xASSERT(0 == (ulConfig & ~(xWDT_INT_FUNCTION | xWDT_RESET_FUNCTION)));

    //
    // Configure Int Type
    //
    xHWREG(ulBase + WDT_O_CTL) &= ~(WDT_CTL_INTEN | WDT_CTL_RESEN);

    xHWREG(ulBase + WDT_O_CTL) |= ulConfig;

    //
    // Set Reload value
    //
    xHWREG(ulBase + WDT_O_LOAD) = ulReload;

}

void xWDTIntCallbackInit(unsigned long ulBase,
                        xtEventCallback xtWDTCallback)
{
    unsigned long WdtBase = 0;
    unsigned long TblSize = 0;
    unsigned long i       = 0;
    //
    // Check the arguments
    //
    xASSERT((ulBase == WATCHDOG0_BASE) || (ulBase == WATCHDOG1_BASE));

    WdtBase = ulBase;
    TblSize = sizeof(g_CallBackTbl) / sizeof(g_CallBackTbl[0]);

    //
    // Find the correct Entry
    //
    for(i = 0; i < TblSize; i++)
    {
        if(WdtBase == g_CallBackTbl[i].Base)
        {
            g_CallBackTbl[i].Fun = xtWDTCallback;
            break;
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

