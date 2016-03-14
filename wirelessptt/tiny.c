tinymix_command("Capture Switch", "1");
tinymix_command("Capture Volume", "60");
tinymix_command("INPGAR IN4R Switch", "1");
tinymix_command("MIXINR PGA Switch" , "1");
tinymix_command("SPKOUTL Mixer MIXINR Switch", "1");

#if 0
tinymix_command("SPKOUTR Mixer MIXINR Switch", "1");
tinymix_command("SPKOUTR PGA", "Mixer");
tinymix_command("Speaker Mixer Switch", "1");
tinymix_command("Speaker Switch", "1");
tinymix_command("Speaker Volume", "121");
#endif
static void set_audio_route(bool enable)
{
	tinymix_command("MIXINR PGA Switch", "0");
#if RFS_AUDIO_DEBUG
	tinymix_command("MIXINL PGA Switch", "0");
#endif

	if (enable) {
#if RFS_USE_IN4R
		tinymix_command("HPMIXL IN4R Switch", "1");
		tinymix_command("SPKOUTL Mixer IN4R Switch", "1");
		//
		tinymix_command("SPKOUTR Mixer IN4R Switch", "1");
#else
		tinymix_command("MIXINR IN3R Switch", "1");
		tinymix_command("HPMIXL MIXINR Switch", "1");
		tinymix_command("SPKOUTL Mixer MIXINR Switch", "1");
#endif

#if RFS_AUDIO_DEBUG
		tinymix_command("MIXINL IN2L Switch", "1");
		tinymix_command("HPMIXL MIXINL Switch", "1");
		tinymix_command("SPKOUTL Mixer MIXINL Switch", "1");
#endif

		tinymix_command("HPOUTL PGA", "Mixer");
		tinymix_command("Headphone Mixer Switch", "1");
		tinymix_command("Headphone Volume", "121");
		tinymix_command("Headphone Switch", "1");

		tinymix_command("SPKOUTL PGA", "Mixer");
		//
		tinymix_command("SPKOUTR PGA", "Mixer");
		tinymix_command("Speaker Mixer Switch", "1");
		tinymix_command("Speaker Switch", "1");
		tinymix_command("Speaker Volume", "121");
	} else {
#if RFS_USE_IN4R
		tinymix_command("HPMIXL IN4R Switch", "0");
		tinymix_command("SPKOUTL Mixer IN4R Switch", "0");
#else
		tinymix_command("MIXINR IN3R Switch", "0");
		tinymix_command("HPMIXL MIXINR Switch", "0");
		tinymix_command("SPKOUTL Mixer MIXINR Switch", "0");
#endif

#if RFS_AUDIO_DEBUG
		tinymix_command("MIXINL IN2L Switch", "0");
#endif

		tinymix_command("Headphone Mixer Switch", "0");
		tinymix_command("Headphone Mixer Switch", "0");
		tinymix_command("HPOUTL PGA", "DAC");

		tinymix_command("Speaker Mixer Switch", "0");
		tinymix_command("SPKOUTL PGA", "DAC");
		// tinymix_command("Headphone Volume", "0");
		// tinymix_command("Headphone Switch", "0");
	}
}