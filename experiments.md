# On Experimental branch
cat > EXPERIMENTS.md << 'EOF'
# QUANTXT Experimental Track

**Branch Purpose**: Active development & testing ground. Main stays stable.

## Current Active Experiments (as of May 29 2026)

### 1. Rich Scenario + Calibration Backwards Compatibility
**Status**: In progress / Blocked  
**Date Started**: May 2026  
**Goal**: Expand `Scenario` to include full `CalibParams` while still loading old `.txt` files.  
**Current Issue**: Old TXT format has fewer datapoints → loader fails.  
**Fixes Tried**:
- [ ] Tolerant loader with defaults for missing fields
- [ ] Version header (`# QUANTXT_SCENARIO v2`)
- [ ] Migration script (Python)

**Next Action**: Paste `load_scenario_file_multi()` for review.

### 2. Module Loading System (`MODLOAD.C`)
**Status**: Active  
**Notes**: ...

## Past Experiments (Completed / Merged)
- Architecture cleanup (v1.11)
- ...

EOF
