# ADR-0001: Use the OBS Studio Program projector

Status: accepted
Date: 2026-09-16

## Context

Program Display needs deliberate full-screen Program video without creating a second
renderer. Current OBS exposes `obs_frontend_open_projector()` and accepts the
`StudioProgram` type.

## Decision

v0.1 calls the public frontend API with `StudioProgram` and a freshly resolved
current monitor index. Program Display will not use private `OBSBasic`/`OBSProjector`
types, fork OBS, or render Program independently.

## Consequences

OBS retains rendering and window ownership. The call returns no result or
handle, so Program Display cannot query, close, or prove the projector. Those gaps are
upstream candidates and directly constrain health and reassignment behavior.
