const ptvo_pattern_control = (endpoint) => {
  const control = exposes.switch().withState('state', true, 'Pattern auto-change').withEndpoint(endpoint);
  control.features.push(exposes.numeric(endpoint, ea.ALL).withDescription('Pattern number'));
  return control;
};
