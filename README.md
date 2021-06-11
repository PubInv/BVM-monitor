# BVM-monitor
A monitor for Bag Valve Masks to measure flow (tidal volume) and prevent operator misuse.

## Motivation

Bag Valve Masks are a fundemental tool used by first responders to assiste patients in respiratory distress. A recognized problem is that they can produce
barotrauma and other forms of volumetric trauma. Particularly in stressful situations, they may be overused. 

The basic idea of this project is to build a small electronic gadget that helps an operator use the device correctly by measuring and correcting both 
the tidal volume delivered and the flow.

## History

This idea was created in coversation between Robert L. Read and Dr. Erich Schulz of Australia while working on the [VentOS project](https://gitlab.com/project-ventos/ventos). Public Invention already had the [VentMon](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor), which performed flow measurment and integrated it get Tidal Volume. This was presented to a Rice University Freshman Design Team, called "Mask and You Shall Recieve" TBD: put their names here!

In the course of the 20-21 school year that did a fair amount of research (including finding an earlier project at Pittsburg, apparently now abandonded.) They
took a VentMon and essentially removed features, producign a stripped-down version that could be thought of as a separate device. They successfully moved the 
integration of flow from the Sensirion SFM3X00 sensor into the Arduion code.

The result is a small device with a beeper and digital display (almost a spirometer) that outputs tidal volume and notes a 6-second cadence.
They also did a number of interviews with EMTS to justify the design ideas.

## Current Plan

Our current plan is to build a very small battery-powered device around the Sensirion SFM3X00 device that is pluggable into a standard 22mm airway and 
makes the use a BVM safer. We believe we can iterate on the current design, test further, and gain more validation for this device.

## Goal

We seek to save lives.

Our approach is to produce a free-libre open source design that is good enough that some manufacturing firm (not us) will undertake to make and market this device,
obtaining whatever regulatory approval is needed to do so. We hope such a firm or firms make profits, but we will not allow them to monopolize this work.
That is, we hope Firm A makes this device that it saves lives in the field eventually, but we will not allow Firm A to prevent Firm B from doing the same things.
To that end we will use the [AGPL license](https://www.gnu.org/licenses/agpl-3.0.en.html) for all code, and [ERN strong reciprocal](https://ohwr.org/cern_ohl_s_v2.txt) license for hardware desings, as noted in the [Public Invention Licensing Guidelines](https://github.com/PubInv/PubInv-License-Guidelines).

### Secondary Goal

This device is effectively a spirometer, which could be used as a non-medical spirometer, as used by athletes and wind-instrument players to improve their skils.
We have no objectiong to this device being used for that purpose, but this goal is subordinate to saving lives.

