const ptvo_pattern_shortcut = (endpoint) => {
  const control = exposes.switch().withState('state', true, 'Activate pattern').withEndpoint(endpoint);
  control.features.push(exposes.numeric(endpoint, ea.ALL).withDescription('Pattern number'));
  return control;
};
