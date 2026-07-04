package main

import (
	"strconv"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
)

func main() {
	a := app.New()
	w := a.NewWindow("Enter your age")
	w.Resize(fyne.NewSize(505, 55))

	ageEntry := widget.NewEntry()
	ageEntry.SetText("0")

	slider := widget.NewSlider(0, 130)
	slider.Step = 1

	slider.OnChanged = func(v float64) {
		ageEntry.SetText(strconv.Itoa(int(v)))
	}

	ageEntry.OnChanged = func(s string) {
		if v, err := strconv.Atoi(s); err == nil {
			if v < 0 {
				v = 0
			}
			if v > 130 {
				v = 130
			}
			slider.SetValue(float64(v))
		}
	}

	// Force entry width
	entryWrap := container.NewGridWrap(
		fyne.NewSize(80, ageEntry.MinSize().Height),
		ageEntry,
	)

	content := container.NewBorder(
		nil, nil,
		entryWrap, nil,
		slider,
	)

	w.SetContent(content)
	w.ShowAndRun()
}
