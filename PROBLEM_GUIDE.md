# Schritt-für-Schritt: LeetCode-Problem implementieren

Dieses Dokument beschreibt den systematischen Weg von einem neuen LeetCode-Problem
bis zur fertigen, ins Menü integrierten Lösung in diesem Projekt.

---

## Phase 1 — Problem verstehen (ohne Code)

**Ziel:** Bevor du auch nur eine Klasse öffnest, musst du das Problem vollständig verstanden haben.

1. **Lies die Aufgabe zweimal.** Was sind Eingabe und Ausgabe, welcher Typ genau?
2. **Schreibe 2–3 Beispiele von Hand durch.** Nicht am PC — auf Papier oder im Kopf.
3. **Kläre Edge Cases:**
   - Leere Eingabe / leere Liste?
   - Negative Zahlen, Nullen, Duplikate?
   - Sehr große Zahlen (Overflow möglich)?
   - Einelementige Arrays / Listen?
4. **Formuliere den Algorithmus in einem Satz**, bevor du die Komplexität überlegst.
   > *„Ich gehe einmal durch das Array und speichere jeden Wert in einer Hash Map,
   > um den gesuchten Complement sofort nachschlagen zu können."*

---

## Phase 2 — Algorithmus entwerfen

**Von Brute Force zu optimal — immer in dieser Reihenfolge denken:**

| Stufe | Frage | Beispiel Two Sum |
|---|---|---|
| Brute Force | Was ist die naivste Lösung? | Doppelte for-Schleife → O(n²) |
| Optimierung | Welche Wiederholungen kann ich eliminieren? | Complement lookup → O(n) mit Hash Map |
| Optimal | Ist das die theoretische Untergrenze? | Ja — jedes Element muss mindestens einmal gelesen werden |

**Komplexität abschätzen vor der Implementierung:**
- Jede verschachtelte Schleife über n Elemente = eine Potenz mehr: O(n²), O(n³) ...
- Hash Map lookup/insert: O(1) amortisiert
- Rekursion auf halbierten Input: O(log n)
- Rekursion auf halbierten Input, Arbeit pro Ebene = n: O(n log n)

---

## Phase 3 — Was brauche ich neu?

### Brauche ich eine neue Klasse?

**Neue Klasse** wenn:
- Der Algorithmus eigene Hilfsmethoden hat (fromVector, freeList, ...)
- Eine eigene Datenstruktur definiert wird (ListNode, TreeNode, ...)
- Mehrere eng verwandte Algorithmen zusammengehören

**Keine neue Klasse** (nur freie Funktion) wenn:
- Es eine einzelne, zustandslose Berechnung ist
- Keine eigene Datenstruktur nötig ist

### Brauche ich eine neue Datenstruktur?

Wenn LeetCode eine Struktur vorgibt (ListNode, TreeNode), gehört sie **in den Header der Algorithmusklasse** (`algorithms/Name.h`). Niemals in `ui/`.

### Brauche ich neue UI-Methoden?

Immer ja, für jedes neue Problem:
- Eine `prompt`-Methode in `ProblemPresenter` für die Benutzereingabe
- Eine `present`-Methode in `ProblemPresenter` für die Ergebnisanzeige

---

## Phase 4 — Implementierungsreihenfolge

Immer in dieser Reihenfolge, **Layer für Layer**:

### Schritt 1: Algorithmus-Klasse anlegen

```
algorithms/MeinProblem.h
algorithms/MeinProblem.cpp
```

- `solve()` — die Kernmethode, nimmt Input entgegen, gibt Ergebnis zurück
- `getName()`, `getTimeComplexity()`, `getSpaceComplexity()` — static, hardcoded Strings
- Utility-Helpers falls nötig (fromVector, toVector, freeList, ...)
- **Keine UI-Includes** — null, keiner, niemals

Orientiere dich an `TwoSum.h` / `AddTwoNumbers.h`.

### Schritt 2: Input-Struct definieren

In `ui/ProblemPresenter.h` ein neues Struct anlegen:

```cpp
struct MeinProblemInput {
    // was der User eingibt, z.B.:
    std::string value{"default"};
};
```

Einfach halten — nur rohe Strings oder Zahlen. Die Konvertierung zum
Algorithmus-Format passiert in `main.cpp`.

### Schritt 3: Eingabe-Form (ProblemPresenter)

In `ProblemPresenter.h` deklarieren:
```cpp
[[nodiscard]] std::optional<MeinProblemInput> promptMeinProblemInput();
```

In `ProblemPresenter.cpp` implementieren:
- FTXUI `ScreenInteractive` + `Input` + `Button`
- Validierung mit sinnvoller Fehlermeldung
- `Esc` → `nullopt` zurückgeben
- Vorausgefüllte Standardwerte (Beispiel aus der Aufgabe)

Orientiere dich an `promptAddTwoNumbersInput()`.

### Schritt 4: Ergebnisanzeige (ProblemPresenter)

In `ProblemPresenter.h` deklarieren:
```cpp
void presentMeinProblem(
    /* alle benötigten Daten */,
    const std::string& algorithmName,
    const std::string& timeComplexity,
    const std::string& spaceComplexity
);
```

In `ProblemPresenter.cpp` implementieren:
- Header via `animation.createFancyHeader(...)`
- Problem-Beschreibung
- Eingabe visualisieren
- **Schritt-für-Schritt Walkthrough** — das ist der Lernwert
- Ergebnis mit Verifikation
- Complexity-Block
- Morpheus-Quote

### Schritt 5: Handler in main.cpp

```cpp
static void handleMeinProblem() {
    ProblemPresenter presenter;
    auto input = presenter.promptMeinProblemInput();
    if (!input) return;  // User hat Esc gedrückt

    // Input in Algorithmus-Format konvertieren
    // ...

    MeinProblem algorithm;
    auto result = algorithm.solve(/* ... */);

    // Ressourcen freigeben (falls Raw Pointer)
    // ...

    presenter.presentMeinProblem(/* ... */,
        MeinProblem::getName(),
        MeinProblem::getTimeComplexity(),
        MeinProblem::getSpaceComplexity()
    );
}
```

### Schritt 6: Menü erweitern

In `ui/MenuRenderer.cpp`, Konstruktor:

```cpp
entries_ = {
    "  Two Sum  (#1)",
    "  Add Two Numbers  (#2)",
    "  Mein Problem  (#X)",   // ← NEU, vor "Beenden"
    "  RayCast - Game",
    "  Beenden",
};
```

In `main.cpp`, switch-case:
```cpp
case 2: handleMeinProblem(); break;  // ← NEU (Zahl anpassen!)
case 3: handleRayCaster();   break;  // bisherige Einträge nach unten schieben
```

> **Wichtig:** "Beenden" ist immer der letzte Eintrag. Die Index-Prüfung
> in `MenuRenderer::showMenu()` nutzt `entries_.size() - 1` und passt sich
> automatisch an — du musst dort nichts ändern.

### Schritt 7: Build und Test

```bash
g++ -std=c++20 \
  main.cpp \
  ui/AnimationEngine.cpp \
  ui/MenuRenderer.cpp \
  ui/ProblemPresenter.cpp \
  algorithms/TwoSum.cpp \
  algorithms/AddTwoNumbers.cpp \
  algorithms/MeinProblem.cpp \
  games/RayCaster.cpp \
  -I/opt/homebrew/include \
  -I. \
  -L/opt/homebrew/lib \
  -lftxui-component -lftxui-dom -lftxui-screen \
  -o neo
./neo
```

Testen mit:
- Standardeingabe (vorausgefüllte Werte)
- Edge Cases (Null, leere Eingabe, sehr große Zahlen)
- `Esc` → zurück zum Menü ohne Absturz

---

## Checkliste

```
[ ] Problem zweimal gelesen, Beispiele von Hand durchgespielt
[ ] Edge Cases identifiziert
[ ] Brute-Force-Lösung gedacht, dann optimiert
[ ] Komplexität (Zeit + Raum) vor dem Coden bestimmt
[ ] algorithms/MeinProblem.h  — keine UI-Includes
[ ] algorithms/MeinProblem.cpp — solve() + Utilities
[ ] struct MeinProblemInput in ProblemPresenter.h
[ ] promptMeinProblemInput() deklariert und implementiert
[ ] presentMeinProblem() deklariert und implementiert (inkl. Walkthrough)
[ ] handleMeinProblem() in main.cpp
[ ] Menü-Eintrag in MenuRenderer.cpp
[ ] case X in main.cpp switch
[ ] Build erfolgreich
[ ] Alle Edge Cases getestet
```

---

## Architektur-Regeln (nicht brechen)

| Regel | Begründung |
|---|---|
| `algorithms/` hat null UI-Includes | Algorithmen müssen isoliert testbar bleiben |
| `ui/` kennt keine Algorithmus-Interna | Presenter nur über öffentliche Interfaces |
| `main.cpp` ist die einzige Kreuzungsschicht | Klare Trennung, einfaches Refactoring |
| Raw Pointer in Algorithmen immer manuell freigeben | `new` und `delete` müssen sich entsprechen |
| `using namespace ftxui` nur in `.cpp`-Dateien | Namespace-Pollution in Headers vermeiden |
