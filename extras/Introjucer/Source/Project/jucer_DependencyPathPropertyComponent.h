/*
  ==============================================================================

    jucer_GlobalDefaultedTextPropertyComponent.h
    Created: 27 Jul 2015 10:42:17am
    Author:  Joshua Gerrard

  ==============================================================================
*/

#ifndef JUCER_DEPENDENCYPATHPROPERTYCOMPONENT_H_INCLUDED
#define JUCER_DEPENDENCYPATHPROPERTYCOMPONENT_H_INCLUDED

//==============================================================================
class DependencyPath
{
public:
    enum OS
    {
        windows = 0,
        osx,
        linux,
        unknown
    };

    static OS getThisOS()
    {
       #if JUCE_WINDOWS
        return DependencyPath::windows;
       #elif JUCE_MAC
        return DependencyPath::osx;
       #elif JUCE_LINUX
        return DependencyPath::linux;
       #else
        return DependencyPath::unknown;
       #endif
    }

    const static String vst2KeyName, vst3KeyName, rtasKeyName, aaxKeyName,
                        androidSdkKeyName, androidNdkKeyName;
};

typedef DependencyPath::OS DependencyPathOS;

//==============================================================================
/** This ValueSource type implements the fallback logic required for dependency
    path settings: use the project exporter value; if this is empty, fall back to
    the global preference value; if the exporter is supposed to run on another
    OS and we don't know what the global preferences on that other machine are,
    fall back to a generic OS-specific fallback value.
*/
class DependencyPathValueSource : public Value::ValueSource,
                                  private Value::Listener
{
public:
    DependencyPathValueSource (const Value& projectSettingsPath,
                               const Value& globalSettingsPath,
                               const String& fallbackPath,
                               DependencyPathOS osThisSettingAppliesTo)
      : projectSettingsValue (projectSettingsPath),
        globalSettingsValue (globalSettingsPath),
        fallbackValue (fallbackPath),
        os (osThisSettingAppliesTo)
    {
        globalSettingsValue.addListener (this);
    }

    /** This gets the currently used value, which may be either
        the project setting, the global setting, or the fallback value. */
    var getValue() const override
    {
        if (isUsingProjectSettings())
            return projectSettingsValue;

        if (isUsingGlobalSettings())
            return globalSettingsValue;

        return fallbackValue;
    }

    void setValue (const var& newValue) override
    {
        projectSettingsValue = newValue;

        if (isUsingProjectSettings())
            sendChangeMessage (false);
    }

    bool isUsingProjectSettings() const
    {
        return projectSettingsValueIsValid();
    }

    bool isUsingGlobalSettings() const
    {
        return ! projectSettingsValueIsValid() && globalSettingsValueIsValid();
    }

    bool isUsingFallbackValue() const
    {
        return ! projectSettingsValueIsValid() && !globalSettingsValueIsValid();
    }

    bool appliesToThisOS() const
    {
        return os == DependencyPath::getThisOS();
    }

private:
    void valueChanged (Value& value) override
    {
        if ((value.refersToSameSourceAs (globalSettingsValue) && isUsingGlobalSettings()))
        {
            sendChangeMessage (true);
            setValue (String::empty); // make sure that the project-specific value is still blank
        }
    }

    /** This defines when to use the project setting, and when to
        consider it invalid and to fall back to the global setting or
        the fallback value. */
    bool projectSettingsValueIsValid() const
    {
        return ! projectSettingsValue.toString().isEmpty();
    }

    /** This defines when to use the global setting - given the project setting
        is invalid, and when to fall back to the fallback value instead. */
    bool globalSettingsValueIsValid() const
    {
        // only use the global settings if they are set on the same OS
        // that this setting is for!
        DependencyPathOS thisOS = DependencyPath::getThisOS();

        return thisOS == DependencyPath::unknown ? false : os == thisOS;
    }

    /** the dependency path setting as set in this Introjucer project. */
    Value projectSettingsValue;

    /** the dependency path global setting on this machine.
        used when there value set for this project is invalid. */
    Value globalSettingsValue;

    /** the dependency path fallback setting. used instead of the global setting
        whenever the latter doesn't apply, e.g. the setting is for another
        OS than the ome this machine is running. */
    String fallbackValue;

    /** on what operating system should this dependency path be used?
        note that this is *not* the os that is targeted by the project,
        but rather the os on which the project will be compiled
        (= on which the path settings need to be set correctly). */
    DependencyPathOS os;
};


//==============================================================================
class DependencyPathPropertyComponent : public TextPropertyComponent,
                                        private Value::Listener,
                                        private Label::Listener
{
public:
    DependencyPathPropertyComponent (const Value& value,
                                     const String& propertyName,
                                     const String& globalKey,
                                     DependencyPathOS os = DependencyPath::getThisOS());


private:
    /** This function defines what colour the label text should assume
        depending on the current state of the value the component tracks. */
    Colour getTextColourToDisplay() const;

    /** This function handles path changes because of user input. */
    void textWasEdited() override;

    /** This function handles path changes because the global path changed. */
    void valueChanged (Value& value) override;

    /** Check if the current value is a valid path. */
    bool isValidPath() const;

    /** the property key of the global property that this component is tracking. */
    String globalKey;

    /** the value source of this dependency path setting. */
    DependencyPathValueSource* pathValueSource;

    /** the value object around the value source. */
    Value pathValue;

    // Label::Listener overrides:
    void labelTextChanged (Label* labelThatHasChanged) override;
    void editorShown (Label*, TextEditor&) override;
    void editorHidden (Label*, TextEditor&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DependencyPathPropertyComponent)
};


#endif  // JUCER_DEPENDENCYPATHPROPERTYCOMPONENT_H_INCLUDED
