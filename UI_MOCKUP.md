# Brew Profiles UI Mockup

## Page Layout

```
┌──────────────────────────────────────────────────────────────────────┐
│ CleverCoffee Logo  [Home] [Profiles] [Settings] [Hardware] [System] │
└──────────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────┐
│                                                                       │
│  Brew Profiles                          [💾 Save Current Settings]   │
│                                                                       │
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐  │
│  │ Light Roast   [⋮]│  │ Dark Espresso [⋮]│  │ Decaf         [⋮]│  │
│  │ ★ ACTIVE         │  │                  │  │                  │  │
│  │                  │  │                  │  │                  │  │
│  │ Temp: 94°C       │  │ Temp: 92°C       │  │ Temp: 96°C       │  │
│  │ Pre-Inf: Yes     │  │ Pre-Inf: Yes     │  │ Pre-Inf: No      │  │
│  │ Mode: Auto       │  │ Mode: Auto       │  │ Mode: Auto       │  │
│  │                  │  │                  │  │                  │  │
│  │ [▶ Load Profile] │  │ [▶ Load Profile] │  │ [▶ Load Profile] │  │
│  └──────────────────┘  └──────────────────┘  └──────────────────┘  │
│                                                                       │
│  ┌──────────────────┐  ┌──────────────────┐                         │
│  │ Lungo         [⋮]│  │    Empty Slot    │                         │
│  │                  │  │                  │                         │
│  │                  │  │        ⊕         │                         │
│  │ Temp: 93°C       │  │   Empty Slot     │                         │
│  │ Pre-Inf: Yes     │  │                  │                         │
│  │ Mode: Manual     │  │ [Save Here]      │                         │
│  │                  │  │                  │                         │
│  │ [▶ Load Profile] │  │                  │                         │
│  └──────────────────┘  └──────────────────┘                         │
│                                                                       │
└──────────────────────────────────────────────────────────────────────┘
```

## Profile Card States

### Active Profile (with green border)
```
┌════════════════════════════════════┐
║ Light Roast                     [⋮]║
║ ★ ACTIVE (green badge)             ║
║                                    ║
║ Temp: 94°C                         ║
║ Pre-Inf: Yes                       ║
║ Mode: Auto                         ║
║                                    ║
║ [▶ Load Profile] (button)          ║
└════════════════════════════════════┘
  ^--- Green border indicates active
```

### Inactive Profile (normal)
```
┌────────────────────────────────────┐
│ Dark Espresso                   [⋮]│
│                                    │
│                                    │
│ Temp: 92°C                         │
│ Pre-Inf: Yes                       │
│ Mode: Auto                         │
│                                    │
│ [▶ Load Profile] (button)          │
└────────────────────────────────────┘
```

### Empty Slot (dashed border)
```
┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┐
│         Empty Slot                │
│                                   │
│            ⊕                      │
│       Empty Slot                  │
│                                   │
│    [Save Here] (button)           │
│                                   │
└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘
  ^--- Dashed border, faded opacity
```

## Dropdown Menu (when clicking ⋮)
```
┌────────────────────┐
│ Rename             │
│ Delete (red text)  │
└────────────────────┘
```

## Save Dialog Modal
```
┌─────────────────────────────────────────────────────────┐
│ Save Current Settings                              [×]  │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Profile Name:                                          │
│  ┌─────────────────────────────────────────────────┐   │
│  │ e.g. Light Roast                                │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  Save to Slot:                                          │
│  ┌─────────────────────────────────────────────────┐   │
│  │ Slot 1 - Light Roast                       ▼   │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│                    [Cancel]  [Save]                     │
└─────────────────────────────────────────────────────────┘
      Dark overlay background (50% transparent)
```

## Color Scheme

- **Active Profile Border:** #28a745 (Green)
- **Active Badge:** Green background, white text
- **Normal Card:** White background, light gray border
- **Empty Slot:** Dashed gray border, 60% opacity
- **Buttons:** Bootstrap primary blue
- **Delete Option:** #dc3545 (Red text)
- **Icons:** Font Awesome
  - ⋮ (ellipsis-v) - Menu icon
  - ▶ (play) - Load icon
  - 💾 (save) - Save icon
  - ⊕ (plus-circle) - Add icon
  - ★ (badge) - Active indicator

## Responsive Behavior

**Desktop (>768px):**
- 3 columns grid
- Cards displayed side-by-side

**Tablet (768px):**
- 2 columns grid
- Cards stacked in pairs

**Mobile (<768px):**
- 1 column grid
- Full-width cards
- Stacked vertically

## Interaction Flows

### Load Profile
1. User clicks anywhere on profile card OR clicks "Load Profile" button
2. POST request to `/profiles/load` with index
3. Alert message: "Profile '{name}' loaded!"
4. Page refreshes profile data
5. Newly loaded profile gets green border and ACTIVE badge

### Save New Profile
1. User clicks "Save Current Settings" button (top right)
2. Modal dialog appears with overlay
3. User enters profile name (e.g., "Light Roast Morning")
4. User selects slot from dropdown (shows slot number and current name)
5. User clicks "Save"
6. POST request to `/profiles/save`
7. Alert: "Profile saved successfully!"
8. Modal closes
9. Profile grid refreshes
10. New/updated profile shown with data

### Rename Profile
1. User clicks ⋮ menu on profile card
2. Dropdown appears with "Rename" and "Delete"
3. User clicks "Rename"
4. Browser prompt: "Enter new name: [current name]"
5. User enters new name
6. POST request to `/profiles/rename`
7. Profile card updates with new name

### Delete Profile
1. User clicks ⋮ menu on profile card
2. User clicks "Delete" (red text)
3. Browser confirm: "Delete profile '{name}'?"
4. User confirms
5. POST request to `/profiles/delete`
6. Card changes to "Empty Slot" state

## Animation & Transitions

- **Card Hover:** Slight lift effect (translateY -5px), shadow appears
- **Button Hover:** Slight color darkening
- **Modal:** Fade in/out with background overlay
- **Profile Switch:** Smooth transition of active indicator
- **All transitions:** 0.3s ease

## Accessibility

- Semantic HTML structure
- ARIA labels for screen readers
- Keyboard navigation support
- Focus indicators on interactive elements
- Color contrast meets WCAG AA standards
- Error messages announced to screen readers

## Error States

**Invalid Name:**
```
Alert: "Please enter a profile name"
```

**Load Failed:**
```
Alert: "Failed to load profile"
```

**Save Failed:**
```
Alert: "Failed to save profile"
```

**Connection Error:**
```
Alert: "Failed to load profiles"
```

## Loading States

While fetching profiles:
```
┌────────────────────────────────────┐
│                                    │
│       Loading profiles...          │
│       ⟳ (spinner)                  │
│                                    │
└────────────────────────────────────┘
```

## Visual Hierarchy

1. **Primary Action:** "Save Current Settings" button (prominent, top-right)
2. **Profile Cards:** Equal visual weight, grid layout
3. **Active Profile:** Stands out with green border and badge
4. **Secondary Actions:** Menu (⋮), Load buttons within cards
5. **Empty Slots:** Less prominent (faded, dashed border)

## Typography

- **Page Title:** H2, bold
- **Profile Name:** H5, bold
- **Details:** Small text, regular weight
- **Buttons:** Medium text, bold
- **Badge:** Small text, uppercase

---

This mockup represents the visual design and user experience of the Brew Profiles feature UI.
The implementation uses Bootstrap 5 and Vue.js 3 for a modern, responsive interface.
